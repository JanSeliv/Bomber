// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "MyViewModelUtilsLibrary.generated.h"

enum class EEndGameState : uint8;
enum class ECurrentGameState : uint8;
enum class ESlateVisibility : uint8;

/**
 * Contains game-specific utility functions to work with UI view models.
 * Is used a lot by the UI View Models as 'Conversion Functions'.
 */
UCLASS()
class BOMBER_API UMyViewModelUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Is used to show or hide own widget by the current game state.
	 * E.g: used by HUD's canvas to have is visible only during GameStarting+InGame states.
	 * @param GameStateProperty Provide the current game state to check. Enum is const-ref to require in blueprints select a property, but not default value.
	 * @param ByGameStates Select one or multiple game states to check.
	 * @return 'Visible' if the 'CurrentGameState' is in the 'GameStates', otherwise 'Collapsed'. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintAutocast))
	static ESlateVisibility GetVisibilityByGameState(
		const ECurrentGameState& GameStateProperty, /*const ref to hide default*/
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.ECurrentGameState")) int32 ByGameStates);
};