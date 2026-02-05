// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMBaseSubsystem.h"

// NMM
#include "Data/NMMDataAsset.h"
#include "NMMUtils.h"

// Bomber
#include "GameFramework/BmrGameState.h"
#include "Structures/BmrGameplayTags.h"
#include "Structures/BmrGameStateTag.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"

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
void UNMMBaseSubsystem::SetNewMainMenuState(ENMMState NewState)
{
	const ENMMState PreviousState = CurrentMenuState;
	CurrentMenuState = NewState;

	OnMainMenuStateChanged.Broadcast(NewState, PreviousState);
}

/*********************************************************************************************
 * Data Asset
 ********************************************************************************************* */

// Returns the data asset that contains all the assets and tweaks of New Main Menu game feature
const UNMMDataAsset* UNMMBaseSubsystem::GetNewMainMenuDataAsset() const
{
	return UMyPrimaryDataAsset::GetOrLoadOnce(NewMainMenuDataAsset);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Is called when the world is initialized
void UNMMBaseSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Clears all transient data contained in this subsystem
void UNMMBaseSubsystem::Deinitialize()
{
	UMyPrimaryDataAsset::ResetDataAsset(NewMainMenuDataAsset);

	Super::Deinitialize();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the current game state was changed, handles Main Menu states accordingly
void UNMMBaseSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		// Player returns to the Main Menu, means Main Menu is in Idle state
		SetNewMainMenuState(ENMMState::Idle);
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		// Player left the Main Menu
		SetNewMainMenuState(ENMMState::None);
	}
}
