// Copyright (c) Yevhenii Selivanov

#include "NMMUtils.h"

// NMM
#include "Components/NMMHUDComponent.h"
#include "Components/NMMPlayerControllerComponent.h"
#include "DalSubsystem.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "Data/NmmStateTag.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "DataRegistries/BmrCinematicRow.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/SlateWrapperTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "MovieSceneSequencePlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMUtils)

/*********************************************************************************************
 * Object getters
 ********************************************************************************************* */

// Returns Main Menu subsystem that provides access to the most important data like Data Asset and current state
UNMMBaseSubsystem* UNMMUtils::GetBaseSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNMMBaseSubsystem>() : nullptr;
}

// Returns Main Menu subsystem that handles menu spots
UNMMSpotsSubsystem* UNMMUtils::GetSpotsSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNMMSpotsSubsystem>() : nullptr;
}

// Returns Main Menu subsystem that handles In-Game Settings which are tweaked by player in Settings menu during the game
UNMMInGameSettingsSubsystem* UNMMUtils::GetInGameSettingsSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	return GEngine ? GEngine->GetEngineSubsystem<UNMMInGameSettingsSubsystem>() : nullptr;
}

// Returns Main Menu subsystem than manages camera possessing and transitions in the Main Menu
UNMMCameraSubsystem* UNMMUtils::GetCameraSubsystem(const UObject* OptionalWorldContext)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNMMCameraSubsystem>() : nullptr;
}

// Returns the Data Asset of the Main Menu
const UNMMDataAsset* UNMMUtils::GetDataAsset()
{
	return UDalSubsystem::GetDataAsset<UNMMDataAsset>();
}

// Returns the HUD component of the Main Menu
UNMMHUDComponent* UNMMUtils::GetHUDComponent(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState(OptionalWorldContext);
	return MyGameState ? MyGameState->FindComponentByClass<UNMMHUDComponent>() : nullptr;
}

// Returns the Player Controller component of the Main Menu
UNMMPlayerControllerComponent* UNMMUtils::GetPlayerControllerComponent(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController(OptionalWorldContext);
	return MyPC ? MyPC->FindComponentByClass<UNMMPlayerControllerComponent>() : nullptr;
}

// Returns the widget of the Main Menu.
UNewMainMenuWidget* UNMMUtils::GetMainMenuWidget(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent(OptionalWorldContext);
	return HUDComponent ? HUDComponent->GetMainMenuWidget() : nullptr;
}

// Returns the widget of the In Cinematic State
UNMMCinematicStateWidget* UNMMUtils::GetInCinematicStateWidget(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent(OptionalWorldContext);
	return HUDComponent ? HUDComponent->GetInCinematicStateWidget() : nullptr;
}

// Returns the Save Game data of the Main Menu
UNMMSaveGameData* UNMMUtils::GetSaveGameData(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UNMMPlayerControllerComponent* MenuControllerComp = GetPlayerControllerComponent(OptionalWorldContext);
	return MenuControllerComp ? MenuControllerComp->GetSaveGameData() : nullptr;
}

/*********************************************************************************************
 * Public API
 ********************************************************************************************* */

// Returns the current state of the Main Menu
FNmmStateTag UNMMUtils::GetMainMenuState()
{
	const UNMMBaseSubsystem* BaseSubsystem = GetBaseSubsystem();
	return BaseSubsystem ? BaseSubsystem->GetCurrentMenuState() : FNmmStateTag::None;
}

/*********************************************************************************************
 * UI
 ********************************************************************************************* */

// Is used to show or hide own widget by the current menu state tag
ESlateVisibility UNMMUtils::GetVisibilityByMenuStateTag(const FNmmStateTag& CurrentMenuStateTag, const FGameplayTagContainer& MenuStates, ESlateVisibility MatchedVisibility /* = ESlateVisibility::SelfHitTestInvisible */)
{
	const bool bMatching = CurrentMenuStateTag.GetSingleTagContainer().HasAny(MenuStates);
	return bMatching ? MatchedVisibility : ESlateVisibility::Collapsed;
}

/*********************************************************************************************
 * Cinematic helpers
 ********************************************************************************************* */

// Returns true if given cinematic wants to skip
bool UNMMUtils::ShouldSkipCinematic(const FBmrCinematicRow& CinematicRow)
{
	if (UMultiplayerUtilsLibrary::IsMultiplayerGame())
	{
		// According design, all the cinematics are available only in single player game
		// while in multiplayer have to skip all of them for all players
		return true;
	}

	// If 'Auto Skip Cinematics' setting is enabled
	const bool bAutoSkipCinematicsSetting = UNMMInGameSettingsSubsystem::Get().IsAutoSkipCinematicsEnabled();

	// If given cinematic has been seen already
	const UNMMSaveGameData* SaveGameData = GetSaveGameData();
	const bool bHasCinematicBeenPlayed = SaveGameData ? SaveGameData->HasCinematicBeenSeen(CinematicRow.Priority) : false;

	// Respect enabled Skip setting if only cinematic has been seen already
	if (bAutoSkipCinematicsSetting && bHasCinematicBeenPlayed)
	{
		return true;
	}

	// --- Put here any other conditions to skip cinematic
	return false;
}

// Helper to initialize playback settings once
static FMovieSceneSequencePlaybackSettings InitPlaybackSettings(FNmmStateTag MenuState)
{
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.LoopCount.Value = MenuState == FNmmStateTag::Idle ? INDEX_NONE : 0; // Loop infinitely if idle, otherwise play once
	Settings.bPauseAtEnd = true; // Pause at the end, so gameplay camera can blend-out from correct position
	Settings.bDisableCameraCuts = true; // Let the Spot to control the camera possessing instead of auto-possessed one that prevents blend-out while active
	const bool bRestoreState = MenuState != FNmmStateTag::Cinematic; // Reset all 'Keep States' tracks when entered to None or Idle states
	Settings.FinishCompletionStateOverride = bRestoreState ? EMovieSceneCompletionModeOverride::ForceRestoreState : EMovieSceneCompletionModeOverride::None;
	return Settings;
}

// Returns the Playback Settings by given cinematic state
const FMovieSceneSequencePlaybackSettings& UNMMUtils::GetCinematicSettings(FNmmStateTag MenuState)
{
	static const FMovieSceneSequencePlaybackSettings EmptySettings = InitPlaybackSettings(FNmmStateTag::None);
	static const FMovieSceneSequencePlaybackSettings IdlePartSettings = InitPlaybackSettings(FNmmStateTag::Idle);
	static const FMovieSceneSequencePlaybackSettings MainPartSettings = InitPlaybackSettings(FNmmStateTag::Cinematic);

	if (MenuState == FNmmStateTag::Idle)
	{
		return IdlePartSettings;
	}

	if (MenuState == FNmmStateTag::Cinematic)
	{
		return MainPartSettings;
	}

	return EmptySettings;
}

// Returns the total frames of the cinematic by given cinematic state
int32 UNMMUtils::GetCinematicTotalFrames(FNmmStateTag MenuState, const UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	const UMovieSceneSequence* LevelSequenceTemplate = LevelSequencePlayer->GetSequence();
	if (MenuState == FNmmStateTag::Idle)
	{
		// Obtain the first subsequence of the Master sequence or null if not found
		LevelSequenceTemplate = UCinematicUtils::FindSubsequence(/*Index*/ 0, LevelSequenceTemplate);
	}

	return UCinematicUtils::GetSequenceTotalFrames(LevelSequenceTemplate);
}

// Return the playback position params by given cinematic state
FMovieSceneSequencePlaybackParams UNMMUtils::GetPlaybackPositionParams(FNmmStateTag MenuState, const UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	FMovieSceneSequencePlaybackParams InPlaybackParams;
	InPlaybackParams.Frame.FrameNumber.Value = [MenuState, LevelSequencePlayer]
	{
		if (MenuState == FNmmStateTag::None)
		{
			// Moving to the last frame will stop the cinematic while regular 'Stop' does not work for clients
			return UCinematicUtils::GetSequenceTotalFrames(LevelSequencePlayer->GetSequence()) - 1;
		}

		if (MenuState == FNmmStateTag::Idle)
		{
			// Start from the beginning
			return 0;
		}

		if (MenuState == FNmmStateTag::Cinematic)
		{
			// Continue from the current frame
			return LevelSequencePlayer->GetCurrentTime().Time.FrameNumber.Value;
		}

		return -1;
	}();

	// Scrub instead of Play\Jump, so it stop currently playing tracks before moving to the new position
	InPlaybackParams.UpdateMethod = EUpdatePositionMethod::Scrub;

	return InPlaybackParams;
}
