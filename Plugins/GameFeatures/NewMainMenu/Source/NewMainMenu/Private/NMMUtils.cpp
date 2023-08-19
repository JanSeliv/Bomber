// Copyright (c) Yevhenii Selivanov

#include "NMMUtils.h"
//---
#include "UI/MyHUD.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "Components/NMMHUDComponent.h"
#include "Data/NMMTypes.h"
#include "MovieSceneSequencePlaybackSettings.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMUtils)

// Returns the HUD component of the Main Menu
UNMMHUDComponent* UNMMUtils::GetHUDComponent()
{
	const AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD();
	return MyHUD ? MyHUD->FindComponentByClass<UNMMHUDComponent>() : nullptr;
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
