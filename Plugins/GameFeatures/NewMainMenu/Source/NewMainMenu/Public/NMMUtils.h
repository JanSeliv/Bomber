// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "NMMUtils.generated.h"

enum class ESlateVisibility : uint8;

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
	/** Returns Main Menu subsystem that provides access to the most important data like Data Asset and current state. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", DisplayName = "Get NMM Base Subsystem", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMBaseSubsystem* GetBaseSubsystem(const UObject* OptionalWorldContext = nullptr);

	/** Returns Main Menu subsystem that handles menu spots. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", DisplayName = "Get NMM Spots Subsystem", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMSpotsSubsystem* GetSpotsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/** Returns Main Menu subsystem that handles In-Game Settings which are tweaked by player in Settings menu during the game. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", DisplayName = "Get NMM In-Game Settings Subsystem", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMInGameSettingsSubsystem* GetInGameSettingsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/** Returns Main Menu subsystem than manages camera possessing and transitions in the Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", DisplayName = "Get NMM Camera Subsystem", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMCameraSubsystem* GetCameraSubsystem(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Data Asset of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM Data Asset")
	static const class UNMMDataAsset* GetDataAsset();

	/** Returns the HUD component of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM HUD Component", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMHUDComponent* GetHUDComponent(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Player Controller component of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM Player Controller Component", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMPlayerControllerComponent* GetPlayerControllerComponent(const UObject* OptionalWorldContext = nullptr);

	/** Returns the widget of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM Main Menu Widget", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNewMainMenuWidget* GetMainMenuWidget(const UObject* OptionalWorldContext = nullptr);

	/** Returns the widget of the In Cinematic State. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM In Cinematic State Widget", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMCinematicStateWidget* GetInCinematicStateWidget(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Save Game data of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM Save Game Data", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNMMSaveGameData* GetSaveGameData(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Public API
	 ********************************************************************************************* */
public:
	/** Returns the current state of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NMM Main Menu State")
	static struct FNmmStateTag GetMainMenuState();

	/*********************************************************************************************
	 * UI
	 ********************************************************************************************* */
public:
	/** Is used to show or hide own widget by the current menu state tag.
	 * E.g: used by New Main Menu widget to be visible only during BasicMenu+Idle states.
	 * @param CurrentMenuStateTag Provide the current menu state tag to check.
	 * @param MenuStates Select one or multiple menu state tags to check.
	 * @param MatchedVisibility Visibility to return when the current tag matches any in the container.
	 * @return MatchedVisibility if the current tag matches any in the container, otherwise 'Collapsed'. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", meta = (BlueprintAutocast, AutoCreateRefTerm = "MenuStates"))
	static ESlateVisibility GetVisibilityByMenuStateTag(
	    const struct FNmmStateTag& CurrentMenuStateTag,
	    UPARAM(meta = (Categories = "NMM.State")) const struct FGameplayTagContainer& MenuStates,
	    ESlateVisibility MatchedVisibility);

	/*********************************************************************************************
	 * Cinematic helpers
	 ********************************************************************************************* */
public:
	/** Returns true if given cinematic wants to skip.
	 * It checks game Settings and save game data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	static bool ShouldSkipCinematic(const struct FBmrCinematicRow& CinematicRow);

	/** Returns the Playback Settings by given cinematic state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Cinematic Settings")
	static const struct FMovieSceneSequencePlaybackSettings& GetCinematicSettings(FNmmStateTag MenuState);

	/** Returns the total frames of the cinematic by given cinematic state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Cinematic Total Frames")
	static int32 GetCinematicTotalFrames(FNmmStateTag MenuState, const UMovieSceneSequencePlayer* LevelSequencePlayer);

	/** Return the playback position params by given cinematic state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Playback Position Params")
	static struct FMovieSceneSequencePlaybackParams GetPlaybackPositionParams(FNmmStateTag MenuState, const UMovieSceneSequencePlayer* LevelSequencePlayer);
};
