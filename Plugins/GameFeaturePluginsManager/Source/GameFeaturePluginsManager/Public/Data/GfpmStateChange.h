// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameFeaturesSubsystem.h" // EGameFeatureTargetState

#include "GfpmStateChange.generated.h"

/**
 * Desired state of particular game feature
 */
USTRUCT(BlueprintType)
struct FGfpmStateChange
{
	GENERATED_BODY()

	/** The name of the game feature plugin to change state for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Game Feature Plugins Manager]")
	FName GameFeatureName = NAME_None;

	/** The target state to transition the game feature to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Game Feature Plugins Manager]")
	EGameFeatureTargetState TargetState = EGameFeatureTargetState::Active;
};