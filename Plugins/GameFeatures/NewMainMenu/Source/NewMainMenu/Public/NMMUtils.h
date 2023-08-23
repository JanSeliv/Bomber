// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "NMMUtils.generated.h"

enum class ENMMCinematicState : uint8;

class UMovieSceneSequencePlayer;

/**
 * Static helper functions about New Main Menu.
 */
UCLASS(Blueprintable, BlueprintType, DisplayName = "New Main Menu Utils")
class NEWMAINMENU_API UNMMUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Object getters
	 ********************************************************************************************* */
public:
	/** Returns the HUD component of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NMM HUD Component")
	static class UNMMHUDComponent* GetHUDComponent();

	/** Returns the Player Controller component of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NMM Player Controller Component")
	static class UNMMPlayerControllerComponent* GetPlayerControllerComponent();

	/** Returns the widget of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UNewMainMenuWidget* GetMainMenuWidget();

	/** Returns the widget of the In Cinematic State. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NMM In Cinematic State Widget")
	static class UNMMCinematicStateWidget* GetInCinematicStateWidget();

	/** Returns the Save Game data of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NMM Save Game Data")
	static class UNMMSaveGameData* GetSaveGameData();

	/*********************************************************************************************
	 * Cinematic helpers
	 ********************************************************************************************* */
public:
	/** Returns true if given cinematic wants to skip. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool ShouldSkipCinematic(const struct FNMMCinematicRow& CinematicRow);

	/** Returns the Playback Settings by given cinematic state. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Cinematic Settings")
	static const struct FMovieSceneSequencePlaybackSettings& GetCinematicSettings(ENMMCinematicState CinematicState);

	/** Returns the total frames of the cinematic by given cinematic state. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Cinematic Total Frames")
	static int32 GetCinematicTotalFrames(ENMMCinematicState CinematicState, const UMovieSceneSequencePlayer* LevelSequencePlayer);

	/** Return the playback position params by given cinematic state. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Playback Position Params")
	static struct FMovieSceneSequencePlaybackParams GetPlaybackPositionParams(ENMMCinematicState CinematicState, const UMovieSceneSequencePlayer* LevelSequencePlayer);
};
