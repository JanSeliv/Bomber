// Copyright (c) Yevhenii Selivanov

#include "NMMUtils.h"
//---
#include "Components/NMMHUDComponent.h"
#include "Components/NMMPlayerControllerComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "Data/NMMTypes.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"
#include "UI/MyHUD.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "MovieSceneSequencePlaybackSettings.h"
#include "MovieSceneSequencePlayer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMUtils)

// Returns Main Menu subsystem that provides access to the most important data like Data Asset and current state
UNMMBaseSubsystem* UNMMUtils::GetBaseSubsystem(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNMMBaseSubsystem>() : nullptr;
}

// Returns Main Menu subsystem that handles menu spots
UNMMSpotsSubsystem* UNMMUtils::GetSpotsSubsystem(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNMMSpotsSubsystem>() : nullptr;
}

// Returns Main Menu subsystem that handles In-Game Settings which are tweaked by player in Settings menu during the game
UNMMInGameSettingsSubsystem* UNMMUtils::GetInGameSettingsSubsystem(const UObject* OptionalWorldContext/* = nullptr*/)
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
const UNMMDataAsset* UNMMUtils::GetDataAsset(const UObject* OptionalWorldContext)
{
	const UNMMBaseSubsystem* Subsystem = GetBaseSubsystem(OptionalWorldContext);
	return Subsystem ? Subsystem->GetNewMainMenuDataAsset() : nullptr;
}

// Returns the HUD component of the Main Menu
UNMMHUDComponent* UNMMUtils::GetHUDComponent(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD(OptionalWorldContext);
	return MyHUD ? MyHUD->FindComponentByClass<UNMMHUDComponent>() : nullptr;
}

// Returns the Player Controller component of the Main Menu
UNMMPlayerControllerComponent* UNMMUtils::GetPlayerControllerComponent(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController(OptionalWorldContext);
	return MyPC ? MyPC->FindComponentByClass<UNMMPlayerControllerComponent>() : nullptr;
}

// Returns the widget of the Main Menu.
UNewMainMenuWidget* UNMMUtils::GetMainMenuWidget(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent(OptionalWorldContext);
	return HUDComponent ? HUDComponent->GetMainMenuWidget() : nullptr;
}

// Returns the widget of the In Cinematic State
UNMMCinematicStateWidget* UNMMUtils::GetInCinematicStateWidget(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent(OptionalWorldContext);
	return HUDComponent ? HUDComponent->GetInCinematicStateWidget() : nullptr;
}

// Returns the Save Game data of the Main Menu
UNMMSaveGameData* UNMMUtils::GetSaveGameData(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UNMMPlayerControllerComponent* MenuControllerComp = GetPlayerControllerComponent(OptionalWorldContext);
	return MenuControllerComp ? MenuControllerComp->GetSaveGameData() : nullptr;
}

// Returns the current state of the Main Menu
ENMMState UNMMUtils::GetMainMenuState()
{
	const UNMMBaseSubsystem* BaseSubsystem = GetBaseSubsystem();
	return BaseSubsystem ? BaseSubsystem->GetCurrentMenuState() : ENMMState::None;
}

// Returns true if given cinematic wants to skip
bool UNMMUtils::ShouldSkipCinematic(const FNMMCinematicRow& CinematicRow)
{
	if (AMyGameStateBase::Get().IsMultiplayerGame())
	{
		// According design, all the cinematics are available only in single player game
		// while in multiplayer have to skip all of them for all players
		return true;
	}

	// If 'Auto Skip Cinematics' setting is enabled
	const bool bAutoSkipCinematicsSetting = UNMMInGameSettingsSubsystem::Get().IsAutoSkipCinematicsEnabled();

	// If given cinematic has been seen already
	const UNMMSaveGameData* SaveGameData = GetSaveGameData();
	const bool bHasCinematicBeenPlayed = SaveGameData ? SaveGameData->HasCinematicBeenSeen(CinematicRow.RowIndex) : false;

	// Respect enabled Skip setting if only cinematic has been seen already
	if (bAutoSkipCinematicsSetting && bHasCinematicBeenPlayed)
	{
		return true;
	}

	// --- Put here any other conditions to skip cinematic
	return false;
}

// Helper namespace to initialize playback settings once
namespace NMMPlaybackSettings
{
	FMovieSceneSequencePlaybackSettings InitPlaybackSettings(ENMMState MainMenuState)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		Settings.LoopCount.Value = MainMenuState == ENMMState::Idle ? INDEX_NONE : 0; // Loop infinitely if idle, otherwise play once
		Settings.bPauseAtEnd = true; // Pause at the end, so gameplay camera can blend-out from correct position
		Settings.bDisableCameraCuts = true; // Let the Spot to control the camera possessing instead of auto-possessed one that prevents blend-out while active
		Settings.bRestoreState = MainMenuState != ENMMState::Cinematic; // Reset all 'Keep States' tracks when entered to None or Idle states
		return Settings;
	}

	const FMovieSceneSequencePlaybackSettings EmptySettings = InitPlaybackSettings(ENMMState::None);
	const FMovieSceneSequencePlaybackSettings IdlePartSettings = InitPlaybackSettings(ENMMState::Idle);
	const FMovieSceneSequencePlaybackSettings MainPartSettings = InitPlaybackSettings(ENMMState::Cinematic);
}

// Returns the Playback Settings by given cinematic state
const FMovieSceneSequencePlaybackSettings& UNMMUtils::GetCinematicSettings(ENMMState MainMenuState)
{
	switch (MainMenuState)
	{
	case ENMMState::Idle: return NMMPlaybackSettings::IdlePartSettings;
	case ENMMState::Cinematic: return NMMPlaybackSettings::MainPartSettings;
	default: return NMMPlaybackSettings::EmptySettings;
	}
}

// Returns the total frames of the cinematic by given cinematic state
int32 UNMMUtils::GetCinematicTotalFrames(ENMMState MainMenuState, const UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	const UMovieSceneSequence* LevelSequenceTemplate = LevelSequencePlayer->GetSequence();
	if (MainMenuState == ENMMState::Idle)
	{
		// Obtain the first subsequence of the Master sequence or null if not found
		LevelSequenceTemplate = UCinematicUtils::FindSubsequence(/*Index*/0, LevelSequenceTemplate);
	}

	return UCinematicUtils::GetSequenceTotalFrames(LevelSequenceTemplate);
}

// Return the playback position params by given cinematic state
FMovieSceneSequencePlaybackParams UNMMUtils::GetPlaybackPositionParams(ENMMState MainMenuState, const UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	FMovieSceneSequencePlaybackParams InPlaybackParams;
	InPlaybackParams.Frame.FrameNumber.Value = [MainMenuState, LevelSequencePlayer]
	{
		switch (MainMenuState)
		{
		case ENMMState::None:
			// Moving to the last frame will stop the cinematic while regular 'Stop' does not work for clients
			return UCinematicUtils::GetSequenceTotalFrames(LevelSequencePlayer->GetSequence()) - 1;
		case ENMMState::Idle:
			// Start from the beginning
			return 0;
		case ENMMState::Cinematic:
			// Continue from the current frame
			return LevelSequencePlayer->GetCurrentTime().Time.FrameNumber.Value;
		default: return -1;
		}
	}();

	// Scrub instead of Play\Jump, so it stop currently playing tracks before moving to the new position
	InPlaybackParams.UpdateMethod = EUpdatePositionMethod::Scrub;

	return InPlaybackParams;
}
