// Copyright (c) Yevhenii Selivanov

#pragma once

// Bomber
#include "Bomber.h" // ECurrentGameState

#include "MouseVisibilitySettings.generated.h"

/**
 * Contains the settings for mouse visibility.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FMouseVisibilitySettings
{
	GENERATED_BODY()

	/** Default settings */
	static const FMouseVisibilitySettings Invalid;

	/** If true, custom game state will be used instead of default one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse", meta = (ShowOnlyInnerProperties))
	bool bUseCustomGameState = false;

	/** Determines the game state to show or hide the mouse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse", meta = (ShowOnlyInnerProperties, EditCondition = "!bUseCustomGameState", EditConditionHides))
	ECurrentGameState GameState = ECurrentGameState::None;

	/** Determines the custom game state to show or hide the mouse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse", meta = (ShowOnlyInnerProperties, EditCondition = "bUseCustomGameState", EditConditionHides))
	FName CustomGameState = NAME_None;

	/** Determines visibility by default. If set, mouse will be shown, otherwise hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse", meta = (ShowOnlyInnerProperties))
	bool bIsVisible = false;

	/** Set true to hide the mouse if inactive for a while.
	 * To work properly, 'Mouse Move' input action has to be assigned to any input context.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse", meta = (ShowOnlyInnerProperties, EditCondition = "bIsVisible", EditConditionHides))
	bool bHideOnInactivity = false;

	/** Set duration to automatically hide the mouse if inactive for a while. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse", meta = (ShowOnlyInnerProperties, EditCondition = "bIsVisible && bHideOnInactivity", EditConditionHides, ClampMin = "0.0"))
	float SecToAutoHide = -1.f;

	/** Returns true if the visibility settings are valid. */
	bool FORCEINLINE IsValid() const { return GameState != ECurrentGameState::None || (bUseCustomGameState && CustomGameState != NAME_None); }

	/** Returns true if according settings, the mouse can be automatically hidden if inactive for a while. */
	bool FORCEINLINE IsInactivityEnabled() const { return bIsVisible && bHideOnInactivity && SecToAutoHide > 0.f; }

	/** Allows for FindByKey() in TArray. */
	FORCEINLINE bool operator==(ECurrentGameState OtherGameState) const { return GameState == OtherGameState; }
	FORCEINLINE bool operator==(FName OtherCustomGameState) const { return CustomGameState == OtherCustomGameState; }
};