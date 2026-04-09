// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMBaseSubsystem.h"

// NMM
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "DataRegistries/BmrCinematicRow.h"
#include "GameFramework/BmrGameState.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMBaseSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNMMBaseSubsystem& UNMMBaseSubsystem::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	UNMMBaseSubsystem* ThisSubsystem = UNMMUtils::GetBaseSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

/*********************************************************************************************
 * New Main Menu State
 ********************************************************************************************* */

// Applies the new state of New Main Menu game feature
void UNMMBaseSubsystem::SetNewMainMenuState(FNmmStateTag NewState)
{
	CurrentMenuStateTag = NewState;

	FGameplayEventData EventData;
	EventData.EventTag = NmmGameplayTags::Event::MenuStateChanged;
	EventData.InstigatorTags.AddTag(NewState);
	UGlobalMessageSubsystem::BroadcastGlobalMessage(EventData);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Subscribes to game state events
void UNMMBaseSubsystem::OnGameFeatureInitialize_Implementation()
{
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	// Listen for spot readiness to re-evaluate state when spots finish loading after DR rows already arrived
	UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	SpotsSubsystem.OnActiveMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnActiveMenuSpotReady);
}

// Clears all transient data contained in this subsystem
void UNMMBaseSubsystem::OnGameFeatureDeinitialize_Implementation()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	// Clear cached MenuStateChanged so late-binding listeners receive fresh data on next menu load
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::MenuStateChanged);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the current game state was changed, handles Main Menu states accordingly
void UNMMBaseSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		// No rows means no MGF cinematics at all (DR itself loads immediately with no softs), enter BasicMenu immediately
		// Rows exist but spots not ready: stay in current state, OnActiveMenuSpotReady will transition to Idle
		// Spots ready: enter Idle directly
		const UNMMSpotsSubsystem* SpotsSubsystem = UNMMUtils::GetSpotsSubsystem();
		const bool bSpotsReady = SpotsSubsystem && SpotsSubsystem->IsActiveMenuSpotReady();
		if (bSpotsReady)
		{
			SetNewMainMenuState(FNmmStateTag::Idle);
		}
		else if (!FBmrCinematicRow::GetRowsNum())
		{
			SetNewMainMenuState(FNmmStateTag::BasicMenu);
		}
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		// Player left the Main Menu
		SetNewMainMenuState(FNmmStateTag::None);
	}
}

// Called when a cinematic spot finished loading, re-evaluates whether to transition from BasicMenu to Idle
void UNMMBaseSubsystem::OnActiveMenuSpotReady_Implementation(UNMMSpotComponent* /*MainMenuSpotComponent*/)
{
	if (!ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::Menu))
	{
		// Spots loaded before game entered Menu state, OnGameStateChanged will handle transition to Idle once Menu state is reached
		return;
	}

	// DR rows are present and spots just became ready, activate cinematic lobby
	SetNewMainMenuState(FNmmStateTag::Idle);
}
