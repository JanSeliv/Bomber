// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BmrViewModelUtilsLibrary.generated.h"

enum class ESlateVisibility : uint8;

/**
 * Contains game-specific utility functions to work with UI view models.
 * Is used a lot by the UI View Models as 'Conversion Functions'.
 */
UCLASS()
class BOMBER_API UBmrViewModelUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Is used to show or hide own widget by the current game state tag.
	 * E.g: used by HUD's canvas to have is visible only during GameStarting+InGame states.
	 * @param CurrentGameStateTag Provide the current game state tag to check.
	 * @param GameStates Select one or multiple game state tags to check.
	 * @return 'SelfHitTestInvisible' if the current tag matches any in the container, otherwise 'Collapsed'. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (BlueprintAutocast, AutoCreateRefTerm = "GameStates"))
	static ESlateVisibility GetVisibilityByGameStateTag(
	    const struct FBmrGameStateTag& CurrentGameStateTag,
	    UPARAM(meta = (Categories = "GameState")) const struct FGameplayTagContainer& GameStates);

	/** Checks if the current game state tag matches any of the specified tags.
	 * Used to determine whether a widget should be active or inactive.
	 * @param CurrentGameStateTag Provide the current game state tag to check.
	 * @param GameStates Select one or multiple game state tags to check.
	 * @return True if the current tag matches any in the container, otherwise false. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (BlueprintAutocast, AutoCreateRefTerm = "GameStates"))
	static bool IsGameStateTagMatching(
	    const struct FBmrGameStateTag& CurrentGameStateTag,
	    UPARAM(meta = (Categories = "GameState")) const struct FGameplayTagContainer& GameStates);
};