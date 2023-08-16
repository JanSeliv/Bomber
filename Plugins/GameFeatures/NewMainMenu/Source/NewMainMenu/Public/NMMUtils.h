// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "NMMUtils.generated.h"

enum class ENMMCinematicState : uint8;

/**
 * Static helper functions about New Main Menu.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNMMUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the HUD component of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NMM HUD Component")
	static class UNMMHUDComponent* GetHUDComponent();

	/** Returns the widget of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UNewMainMenuWidget* GetMainMenuWidget();

	/** Returns the widget of the In Cinematic State. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NMM In Cinematic State Widget")
	static class UNMMCinematicStateWidget* GetInCinematicStateWidget();

	/** Returns the Playback Settings by given cinematic state. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Cinematic Settings")
	static const struct FMovieSceneSequencePlaybackSettings& GetCinematicSettings(ENMMCinematicState CinematicState);
};
