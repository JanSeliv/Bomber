// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMBaseSubsystem.h"

// NMM
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "DataRegistries/BmrCinematicRow.h"
#include "GameFramework/BmrGameState.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrModularGameFeaturesLoaderSubsystem.h"
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

// Predicts the target menu state based on current cinematic readiness
FNmmStateTag UNMMBaseSubsystem::GetPredictedMenuState() const
{
	// Spots ready: menu is ready with cinematic lobby
	const UNMMSpotsSubsystem* SpotsSubsystem = UNMMUtils::GetSpotsSubsystem();
	const bool bSpotsReady = SpotsSubsystem && SpotsSubsystem->IsActiveMenuSpotReady();
	if (bSpotsReady)
	{
		return FNmmStateTag::Idle;
	}

	// Tag-driven MGF still pending activation, can not validate now
	const UBmrModularGameFeaturesLoaderSubsystem* MgfSubsystem = UBmrModularGameFeaturesLoaderSubsystem::GetModularGameFeaturesSubsystem();
	if (MgfSubsystem && MgfSubsystem->HasPendingTagDrivenActivations())
	{
		return FNmmStateTag::None;
	}

	// No rows means no MGF cinematics at all (DR itself loads immediately with no softs), menu is ready immediately
	if (!FBmrCinematicRow::GetRowsNum())
	{
		return FNmmStateTag::BasicMenu;
	}

	// Rows exist but spots not ready: stay in loading screen, OnActiveMenuSpotReady will re-attempt
	return FNmmStateTag::None;
}

// Broadcasts MenuReady global message if not already broadcast and menu prerequisites are met
void UNMMBaseSubsystem::TryBroadcastMenuReady()
{
	if (UGlobalMessageSubsystem::HasBroadcastedMessage(NmmGameplayTags::Event::MenuReady)
	    || !UGlobalMessageSubsystem::HasBroadcastedMessage(BmrGameplayTags::Event::Player_PawnReady)
	    || GetPredictedMenuState() == FNmmStateTag::None)
	{
		// Already entered menu or pawn is not possessed yet or should wait for cinematics loaded
		return;
	}

	// Broadcast MenuReady to trigger global Menu game state via StateTree
	FGameplayEventData MenuReadyData;
	MenuReadyData.EventTag = NmmGameplayTags::Event::MenuReady;
	UGlobalMessageSubsystem::BroadcastGlobalMessage(MenuReadyData);

	// Forward to server so its StateTree can transition to Menu state as well
	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (MyPC && !MyPC->HasAuthority())
	{
		MyPC->ServerBroadcastMessage(MenuReadyData);
	}
}

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

	// Listen for first pawn to attempt MenuReady broadcast with prediction
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::Player_PawnReady, this, &ThisClass::OnFirstPawnReady);

	// Listen for spot readiness to re-evaluate state when spots finish loading after DR rows already arrived
	UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	SpotsSubsystem.OnActiveMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnActiveMenuSpotReady);
}

// Clears all transient data contained in this subsystem
void UNMMBaseSubsystem::OnGameFeatureDeinitialize_Implementation()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	// Clear cached messages so late-binding listeners receive fresh data on next menu load
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::MenuStateChanged);
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::MenuReady);
}

// Recovers menu state after game feature activations
void UNMMBaseSubsystem::OnGameFeatureActivated(const UGameFeatureData* /*GameFeatureData*/, const FString& /*PluginURL*/)
{
	if (CurrentMenuStateTag != FNmmStateTag::None
	    || !ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::Menu))
	{
		// Not in the wait condition, nothing to recover
		return;
	}

	TryBroadcastMenuReady();

	const FNmmStateTag PredictedState = GetPredictedMenuState();
	if (PredictedState != FNmmStateTag::None)
	{
		// It's likely client raced replicated Menu game state vs local MGF activating, leaving NMM state stuck at None, apply desired state
		SetNewMainMenuState(PredictedState);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the first player character is spawned, possessed, and replicated
void UNMMBaseSubsystem::OnFirstPawnReady_Implementation(const FGameplayEventData& Payload)
{
	TryBroadcastMenuReady();
}

// Called when the current game state was changed, handles Main Menu states accordingly
void UNMMBaseSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		const FNmmStateTag PredictedState = GetPredictedMenuState();
		if (PredictedState != FNmmStateTag::None)
		{
			SetNewMainMenuState(PredictedState);
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
	TryBroadcastMenuReady();

	// Upgrade BasicMenu to Idle now that cinematic spots are available
	// Also handle client case where Menu tag replicated but NMM state is still None (spots weren't ready during OnGameStateChanged)
	if (CurrentMenuStateTag == FNmmStateTag::BasicMenu
	    || (CurrentMenuStateTag == FNmmStateTag::None && ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::Menu)))
	{
		SetNewMainMenuState(FNmmStateTag::Idle);
	}
}
