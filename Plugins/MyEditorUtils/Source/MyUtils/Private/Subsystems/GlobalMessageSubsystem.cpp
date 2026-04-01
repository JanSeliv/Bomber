// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GlobalMessageSubsystem.h"

// MyUtils
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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

// Blueprint-only listener node, wraps CallOrStartListeningForGlobalMessage
FAsyncMessageHandle UGlobalMessageSubsystem::BPCallOrStartListeningForGlobalMessage(UObject* WorldContextObject, FGameplayTag MessageTag, const FOnGlobalMessageReceived& Completed)
{
	return CallOrStartListeningForGlobalMessage(MessageTag, WorldContextObject, [Completed](const FGameplayEventData& Payload)
	{
		Completed.ExecuteIfBound(Payload);
	});
}

// Subscribes to a gameplay event via lambda callback with weak object safety
FAsyncMessageHandle UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, const UObject* ListenerOwner, TFunction<void(const FGameplayEventData&)>&& Callback)
{
	FAsyncMessageHandle Handle;

	if (!ensureMsgf(MessageTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'MessageTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return Handle;
	}

	// Replay all cached payloads for this tag to the late subscriber, one per unique instigator
	const UWorld* World = UUtilsLibrary::GetPlayWorld(ListenerOwner);
	const UGlobalMessageSubsystem* Subsystem = World ? World->GetSubsystem<UGlobalMessageSubsystem>() : nullptr;
	const TMap<TWeakObjectPtr<const AActor>, FGameplayEventData>* CachedPayloads = Subsystem ? Subsystem->BroadcastedMessagesMap.Find(MessageTag) : nullptr;
	if (CachedPayloads)
	{
		for (const TPair<TWeakObjectPtr<const AActor>, FGameplayEventData>& CachedEntry : *CachedPayloads)
		{
			Callback(CachedEntry.Value);
		}
		// Fall through to bind for future broadcasts
	}

	// Register for future broadcasts via engine's Async Message System, wrap with weak safety
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	TWeakObjectPtr WeakOwner(ListenerOwner);
	if (!ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		return Handle;
	}

	Handle = MessageSystem->BindListener(FAsyncMessageId(MessageTag), [WeakOwner, UserCallback = MoveTemp(Callback)](const FAsyncMessage& Message)
	{
		const FGameplayEventData* Payload = WeakOwner.IsValid() ? Message.GetPayloadData<const FGameplayEventData>() : nullptr;
		if (Payload)
		{
			UserCallback(*Payload);
		}
	});

	return Handle;
}

// Unbinds a listener so it will no longer receive callbacks
void UGlobalMessageSubsystem::StopListeningForGlobalMessage(const FAsyncMessageHandle& Handle, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (!Handle.IsValid())
	{
		return;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	if (const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World))
	{
		MessageSystem->UnbindListener(Handle);
	}
}

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
		TMap<TWeakObjectPtr<const AActor>, FGameplayEventData>& CachedPayloadsRef = Subsystem->BroadcastedMessagesMap.FindOrAdd(Payload.EventTag);
		CachedPayloadsRef.Add(Payload.Instigator, Payload);
	}

	// Broadcast via engine's Async Message System
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	if (ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		MessageSystem->QueueMessageForBroadcast(FAsyncMessageId(Payload.EventTag), FConstStructView::Make(Payload));
	}

	// Forward event to Ability System Component if found from Target or Instigator as fallback
	const AActor* ASCOwner = Payload.Target ? Payload.Target : Payload.Instigator;
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ASCOwner))
	{
		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

// Clears cached broadcast data
void UGlobalMessageSubsystem::Deinitialize()
{
	Super::Deinitialize();

	BroadcastedMessagesMap.Empty();
}