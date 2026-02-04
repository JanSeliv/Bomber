// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrGameplayMessageSubsystem.h"

// Bomber
#include "Bomber.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

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
	return World ? UGameInstance::GetSubsystem<UBmrGameplayMessageSubsystem>(World->GetGameInstance()) : nullptr;
}

// Sends a gameplay event globally via Gameplay Message Router, and optionally to ASC if found
void UBmrGameplayMessageSubsystem::BroadcastMessage(const FGameplayEventData& Payload, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (!ensureMsgf(Payload.EventTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'EventTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Broadcast via Gameplay Message Router, so any system can listen without GAS dependency
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	if (ensureMsgf(HasInstance(World), TEXT("ASSERT: [%i] %hs:\n'GameplayMessageSubsystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		UGameplayMessageSubsystem::Get(World).BroadcastMessage<FGameplayEventData>(Payload.EventTag, Payload);
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

// Is called when this Subsystem is removed
void UBmrGameplayMessageSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ReadyHandler.Reset();
}