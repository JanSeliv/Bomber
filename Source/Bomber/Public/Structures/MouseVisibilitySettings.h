// Copyright (c) Yevhenii Selivanov

#pragma once

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
	bool FORCEINLINE IsValid() const { return SecToAutoHide >= 0.f; }

	/** Returns true if according settings, the mouse can be automatically hidden if inactive for a while. */
	bool FORCEINLINE IsInactivityEnabled() const { return bIsVisible && bHideOnInactivity && SecToAutoHide > 0.f; }
};
