// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMBaseSubsystem.h"
//---
#include "NMMUtils.h"
#include "Data/NMMDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMBaseSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNMMBaseSubsystem& UNMMBaseSubsystem::Get(const UObject* OptionalWorldContext/* = nullptr*/)
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
	CurrentMenuStateInternal = NewState;

	OnMainMenuStateChanged.Broadcast(NewState);
}

/*********************************************************************************************
 * Data Asset
 ********************************************************************************************* */

// Returns the data asset that contains all the assets and tweaks of New Main Menu game feature
const UNMMDataAsset* UNMMBaseSubsystem::GetNewMainMenuDataAsset() const
{
	return NewMainMenuDataAssetInternal.LoadSynchronous();
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
	NewMainMenuDataAssetInternal.Reset();

	Super::Deinitialize();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the current game state was changed, handles Main Menu states accordingly
void UNMMBaseSubsystem::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
	case ECurrentGameState::Menu:
		{
			// Player returns to the Main Menu, means Main Menu is in Idle state
			SetNewMainMenuState(ENMMState::Idle);
			break;
		}
	case ECurrentGameState::GameStarting:
		{
			// Player left the Main Menu 
			SetNewMainMenuState(ENMMState::None);
			break;
		}
	default: break;
	}
}
