// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrGameplayMessageSubsystem.h"

// Bomber
#include "Bomber.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AsyncMessageId.h"
#include "AsyncMessageSystemBase.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Engine/World.h"
#include "StructUtils/StructView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameplayMessageSubsystem)

// Returns this Subsystem, is checked and will crash if can't be obtained
UBmrGameplayMessageSubsystem& UBmrGameplayMessageSubsystem::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	UBmrGameplayMessageSubsystem* GameplayMessageRouter = GetGameplayMessageRouter(OptionalWorldContext);
	checkf(GameplayMessageRouter, TEXT("ERROR: [%i] %hs:\n'GameplayMessageRouter' is null!"), __LINE__, __FUNCTION__);
	return *GameplayMessageRouter;
}

UBmrGameplayMessageSubsystem* UBmrGameplayMessageSubsystem::GetGameplayMessageRouter(const UObject* OptionalWorldContext)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UBmrGameplayMessageSubsystem>() : nullptr;
}

// Sends a gameplay event globally via Async Message System (aka Lyra's Gameplay Message Router), and optionally to ASC if found
void UBmrGameplayMessageSubsystem::BroadcastMessage(const FGameplayEventData& Payload, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (!ensureMsgf(Payload.EventTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'EventTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Broadcast as global event, so any system can listen without GAS dependency
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	if (ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		MessageSystem->QueueMessageForBroadcast(FAsyncMessageId(Payload.EventTag), FConstStructView::Make(Payload));
	}

	// Try to find ASC: check Target first, then Instigator as fallback, and forward event to the Ability System
	const AActor* ASCOwner = Payload.Target != nullptr ? Payload.Target : Payload.Instigator;
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ASCOwner))
	{
		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
	else
	{
		UE_LOG(LogBomber, Verbose, TEXT("[%i] %hs: ASC is not found on '%s' for event '%s', skipping HandleGameplayEvent"), __LINE__, __FUNCTION__, *GetNameSafe(ASCOwner), *Payload.EventTag.ToString());
	}
}

// Registers a listener for the given event tag via Async Message System (aka Lyra's Gameplay Message Router)
void UBmrGameplayMessageSubsystem::RegisterListener(const UObject* WorldContext, FGameplayTag EventTag, TFunction<void(const FGameplayEventData&)>&& Callback)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(WorldContext);
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	if (!ensureMsgf(MessageSystem, TEXT("ASSERT: [%i] %hs:\n'MessageSystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	MessageSystem->BindListener(FAsyncMessageId(EventTag), [UserCallback = MoveTemp(Callback)](const FAsyncMessage& Message)
	{
		if (const FGameplayEventData* Payload = Message.GetPayloadData<const FGameplayEventData>())
		{
			UserCallback(*Payload);
		}
	});
}

// Is called when this Subsystem is removed
void UBmrGameplayMessageSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ReadyHandler.Reset();
}