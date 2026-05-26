// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GlobalMessageSubsystem.h"

// MyUtils
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AsyncMessageHandle.h"
#include "AsyncMessageId.h"
#include "AsyncMessageSystemBase.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Engine/World.h"
#include "StructUtils/StructView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalMessageSubsystem)

// Returns this Subsystem, is checked and will crash if can't be obtained
UGlobalMessageSubsystem& UGlobalMessageSubsystem::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	UGlobalMessageSubsystem* Subsystem = GetGlobalMessageSubsystem(OptionalWorldContext);
	checkf(Subsystem, TEXT("ERROR: [%i] %hs:\n'GlobalMessageSubsystem' is null!"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this Subsystem, nullptr if world is not available
UGlobalMessageSubsystem* UGlobalMessageSubsystem::GetGlobalMessageSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
}

/*********************************************************************************************
 * Listeners
 ********************************************************************************************* */

// Blueprint-only listener node, wraps CallOrStartListeningForGlobalMessage
void UGlobalMessageSubsystem::BPCallOrStartListeningForGlobalMessage(UObject* WorldContextObject, FGameplayTag MessageTag, const FOnGlobalMessageReceived& Completed)
{
	CallOrStartListeningForGlobalMessage(MessageTag, WorldContextObject, [Completed](const FGameplayEventData& Payload)
	{
		Completed.ExecuteIfBound(Payload);
	});
}

// Subscribes to a gameplay event via lambda callback with weak object safety
void UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, const UObject* ListenerOwner, TFunction<void(const FGameplayEventData&)>&& Callback)
{
	if (!ensureMsgf(MessageTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'MessageTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld(ListenerOwner);

	// Unbind previous listener for this (owner, tag) pair if one exists
	UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	if (Subsystem)
	{
		TMap<FGameplayTag, FMyListenerEntry>* OwnerEntries = Subsystem->ListenerHandlesMap.Find(ListenerOwner);
		const FMyListenerEntry* ExistingEntry = OwnerEntries ? OwnerEntries->Find(MessageTag) : nullptr;
		if (ExistingEntry && ExistingEntry->Handle.IsValid())
		{
			if (const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World))
			{
				MessageSystem->UnbindListener(ExistingEntry->Handle);
			}
			OwnerEntries->Remove(MessageTag);
		}
	}

	// Replay all cached payloads for this tag to the late subscriber, one per unique instigator
	uint64 LastReplayedFrame = 0;
	const TMap<TWeakObjectPtr<const AActor>, FMyBroadcastedEntry>* CachedPayloads = Subsystem ? Subsystem->BroadcastedMessagesMap.Find(MessageTag) : nullptr;
	if (CachedPayloads)
	{
		const uint64 CurrentFrame = GFrameCounter;
		for (const TPair<TWeakObjectPtr<const AActor>, FMyBroadcastedEntry>& CachedEntry : *CachedPayloads)
		{
			// Skip same-frame entries, engine queue still holds them and will deliver at TG_PostUpdateWork, avoids cache+queue double delivery
			if (CachedEntry.Value.BroadcastFrame < CurrentFrame)
			{
				Callback(CachedEntry.Value.Payload);
				LastReplayedFrame = CurrentFrame;
			}
		}
		// Fall through to bind for future broadcasts
	}

	// Register for future broadcasts via engine's Async Message System, wrap with weak safety
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	TWeakObjectPtr WeakOwner(ListenerOwner);
	if (!ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const FAsyncMessageHandle Handle = MessageSystem->BindListener(FAsyncMessageId(MessageTag), [WeakOwner, WeakSubsystem = TWeakObjectPtr(Subsystem), MessageTag, UserCallback = MoveTemp(Callback)](const FAsyncMessage& Message)
	{
		const FGameplayEventData* Payload = Message.GetPayloadData<const FGameplayEventData>();
		if (!ensureMsgf(Payload, TEXT("ASSERT: [%i] %hs:\n'Payload' is was not provided! MessageTag: %s | Listener: %s"), __LINE__, __FUNCTION__, *MessageTag.ToString(), *GetNameSafe(WeakOwner.Get()))
		    || !WeakOwner.IsValid())
		{
			// Listener owner gone, nothing to broadcast
			return;
		}

		UGlobalMessageSubsystem* StrongSubsystem = WeakSubsystem.Get();
		TMap<FGameplayTag, FMyListenerEntry>* OwnerEntries = StrongSubsystem ? StrongSubsystem->ListenerHandlesMap.Find(WeakOwner) : nullptr;
		FMyListenerEntry* Entry = OwnerEntries ? OwnerEntries->Find(MessageTag) : nullptr;
		if (Entry && Entry->LastReplayedFrame != 0 && Message.GetQueueFrame() <= Entry->LastReplayedFrame)
		{
			// Cache replay already delivered same-or-older broadcast this frame, clear marker so subsequent broadcasts fire normally
			Entry->LastReplayedFrame = 0;
			return;
		}

		UserCallback(*Payload);
	});

	// Store entry internally for unbind-by-owner support and queue-dedup state
	if (Subsystem && Handle.IsValid())
	{
		Subsystem->ListenerHandlesMap.FindOrAdd(ListenerOwner).Add(MessageTag, FMyListenerEntry{Handle, LastReplayedFrame});
	}
}

/*********************************************************************************************
 * Broadcast
 ********************************************************************************************* */

// Broadcasts Gameplay Event Data via engine's Async Message System and caches the event for the CallOr pattern
void UGlobalMessageSubsystem::BroadcastGlobalMessage(const FGameplayEventData& Payload, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (!ensureMsgf(Payload.EventTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'EventTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);

	// Cache for the CallOr pattern, subsystem might be unavailable during shutdown
	UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	if (Subsystem)
	{
		TMap<TWeakObjectPtr<const AActor>, FMyBroadcastedEntry>& CachedPayloadsRef = Subsystem->BroadcastedMessagesMap.FindOrAdd(Payload.EventTag);
		CachedPayloadsRef.Add(Payload.Instigator, FMyBroadcastedEntry{Payload, GFrameCounter});
	}

	// Broadcast via engine's Async Message System
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	if (ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		MessageSystem->QueueMessageForBroadcast(FAsyncMessageId(Payload.EventTag), FConstStructView::Make(Payload));
	}

	// Forward event to Ability System Component if found from Target or Instigator as fallback
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Target);
	UAbilitySystemComponent* FinalASC = TargetASC ? TargetASC : UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Instigator);
	if (FinalASC)
	{
		FinalASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

/*********************************************************************************************
 * Unbind
 ********************************************************************************************* */

// Unbinds a single event for the given listener while keeping all other events this listener is subscribed to
void UGlobalMessageSubsystem::StopListeningForGlobalMessage(FGameplayTag MessageTag, const UObject* ListenerOwner)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(ListenerOwner);
	UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	if (!Subsystem)
	{
		// World is tearing down or not a play world; nothing to unbind
		return;
	}

	TMap<FGameplayTag, FMyListenerEntry>* OwnerEntries = Subsystem->ListenerHandlesMap.Find(ListenerOwner);
	const FMyListenerEntry* FoundEntry = OwnerEntries ? OwnerEntries->Find(MessageTag) : nullptr;
	if (!FoundEntry || !FoundEntry->Handle.IsValid())
	{
		return;
	}

	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	if (ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		MessageSystem->UnbindListener(FoundEntry->Handle);
	}

	OwnerEntries->Remove(MessageTag);
	if (OwnerEntries->IsEmpty())
	{
		Subsystem->ListenerHandlesMap.Remove(ListenerOwner);
	}
}

// Unbinds all events for the given listener at once
void UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(const UObject* ListenerOwner)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(ListenerOwner);
	UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	if (!Subsystem)
	{
		// World is tearing down or not a play world; nothing to unbind
		return;
	}

	TMap<FGameplayTag, FMyListenerEntry>* OwnerEntries = Subsystem->ListenerHandlesMap.Find(ListenerOwner);
	if (!OwnerEntries)
	{
		return;
	}

	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	if (ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		for (const TPair<FGameplayTag, FMyListenerEntry>& Entry : *OwnerEntries)
		{
			if (Entry.Value.Handle.IsValid())
			{
				MessageSystem->UnbindListener(Entry.Value.Handle);
			}
		}
	}

	Subsystem->ListenerHandlesMap.Remove(ListenerOwner);
}

// Clears all cached broadcast data for the given tag, restarting the logical session
void UGlobalMessageSubsystem::ClearCachedMessages(FGameplayTag MessageTag, const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	if (Subsystem)
	{
		Subsystem->BroadcastedMessagesMap.Remove(MessageTag);
	}
}

// Returns true if the given event has already been broadcast at least once this session
bool UGlobalMessageSubsystem::HasBroadcastedMessage(FGameplayTag MessageTag, const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	const UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	const TMap<TWeakObjectPtr<const AActor>, FMyBroadcastedEntry>* CachedPayloads = Subsystem ? Subsystem->BroadcastedMessagesMap.Find(MessageTag) : nullptr;
	return CachedPayloads && !CachedPayloads->IsEmpty();
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Clears cached broadcast data
void UGlobalMessageSubsystem::Deinitialize()
{
	Super::Deinitialize();

	BroadcastedMessagesMap.Empty();
	ListenerHandlesMap.Empty();
}