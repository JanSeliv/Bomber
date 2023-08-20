// Copyright (c) Yevhenii Selivanov

#include "NMMUtils.h"
//---
#include "Components/NMMHUDComponent.h"
#include "Components/NMMPlayerControllerComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMTypes.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "UI/MyHUD.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "MovieSceneSequencePlaybackSettings.h"
#include "MovieSceneSequencePlayer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMUtils)

// Returns the HUD component of the Main Menu
UNMMHUDComponent* UNMMUtils::GetHUDComponent()
{
	const AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD();
	return MyHUD ? MyHUD->FindComponentByClass<UNMMHUDComponent>() : nullptr;
}

// Returns the Player Controller component of the Main Menu
UNMMPlayerControllerComponent* UNMMUtils::GetPlayerControllerComponent()
{
	const AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	return MyPC ? MyPC->FindComponentByClass<UNMMPlayerControllerComponent>() : nullptr;
}

// Returns the widget of the Main Menu.
UNewMainMenuWidget* UNMMUtils::GetMainMenuWidget()
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent();
	return HUDComponent ? HUDComponent->GetMainMenuWidget() : nullptr;
}

// Returns the widget of the In Cinematic State
UNMMCinematicStateWidget* UNMMUtils::GetInCinematicStateWidget()
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent();
	return HUDComponent ? HUDComponent->GetInCinematicStateWidget() : nullptr;
}

// Returns true if given cinematic wants to skip
bool UNMMUtils::ShouldSkipCinematic(const FNMMCinematicRow& CinematicRow)
{
	// According design, all the cinematics are available only in single player game
	const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	const bool bIsMultiplayerGame = MyGameState && MyGameState->IsMultiplayerGame();

	// If 'Auto Skip Cinematics' setting is enabled, then skip all the cinematics 
	const UNMMPlayerControllerComponent* MenuControllerComp = GetPlayerControllerComponent();
	const bool AutoSkipCinematicsSetting = MenuControllerComp ? MenuControllerComp->IsAutoSkipCinematicsSetting() : false;

	// --- Put here any other conditions to skip cinematic
	return bIsMultiplayerGame || AutoSkipCinematicsSetting;
}

// Helper namespace to initialize playback settings once
namespace NMMPlaybackSettings
{
	FMovieSceneSequencePlaybackSettings InitPlaybackSettings(ENMMCinematicState CinematicState)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		Settings.LoopCount.Value = CinematicState == ENMMCinematicState::IdlePart ? INDEX_NONE : 0; // Loop infinitely if idle, otherwise play once
		Settings.bPauseAtEnd = true; // Pause at the end, so gameplay camera can blend-out from correct position
		Settings.bDisableCameraCuts = true; // Let the Spot to control the camera possessing instead of auto-possessed one that prevents blend-out while active
		Settings.bRestoreState = CinematicState != ENMMCinematicState::MainPart; // Reset all 'Keep States' tracks when entered to None or IdlePart states
		return Settings;
	}

	const FMovieSceneSequencePlaybackSettings EmptySettings = InitPlaybackSettings(ENMMCinematicState::None);
	const FMovieSceneSequencePlaybackSettings IdlePartSettings = InitPlaybackSettings(ENMMCinematicState::IdlePart);
	const FMovieSceneSequencePlaybackSettings MainPartSettings = InitPlaybackSettings(ENMMCinematicState::MainPart);
}

// Returns the Playback Settings by given cinematic state
const FMovieSceneSequencePlaybackSettings& UNMMUtils::GetCinematicSettings(ENMMCinematicState CinematicState)
{
	switch (CinematicState)
	{
	case ENMMCinematicState::IdlePart: return NMMPlaybackSettings::IdlePartSettings;
	case ENMMCinematicState::MainPart: return NMMPlaybackSettings::MainPartSettings;
	default: return NMMPlaybackSettings::EmptySettings;
	}
}

// Returns the total frames of the cinematic by given cinematic state
int32 UNMMUtils::GetCinematicTotalFrames(ENMMCinematicState CinematicState, const UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	const UMovieSceneSequence* LevelSequenceTemplate = LevelSequencePlayer->GetSequence();
	if (CinematicState == ENMMCinematicState::IdlePart)
	{
		// Obtain the first subsequence of the Master sequence or null if not found
		LevelSequenceTemplate = UCinematicUtils::FindSubsequence(/*Index*/0, LevelSequenceTemplate);
	}

	return UCinematicUtils::GetSequenceTotalFrames(LevelSequenceTemplate);
}

// Return the playback position params by given cinematic state
FMovieSceneSequencePlaybackParams UNMMUtils::GetPlaybackPositionParams(ENMMCinematicState CinematicState, const UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	FMovieSceneSequencePlaybackParams InPlaybackParams;
	InPlaybackParams.Frame.FrameNumber.Value = [CinematicState, LevelSequencePlayer]
	{
		switch (CinematicState)
		{
		case ENMMCinematicState::None:
			// Moving to the last frame will stop the cinematic while regular 'Stop' does not work for clients
			return UCinematicUtils::GetSequenceTotalFrames(LevelSequencePlayer->GetSequence()) - 1;
		case ENMMCinematicState::IdlePart:
			// Start from the beginning
			return 0;
		case ENMMCinematicState::MainPart:
			// Continue from the current frame
			return LevelSequencePlayer->GetCurrentTime().Time.FrameNumber.Value;
		default: return -1;
		}
	}();

	// Scrub instead of Play\Jump, so it stop currently playing tracks before moving to the new position
	InPlaybackParams.UpdateMethod = EUpdatePositionMethod::Scrub;

	return InPlaybackParams;
}
