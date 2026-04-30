// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameFeaturesSubsystem.h" // EGameFeatureTargetState

#include "GameFeatureStateChange.generated.h"

/**
 * Desired state of particular game feature
 */
USTRUCT(BlueprintType)
struct FGameFeatureStateChange
{
	GENERATED_BODY()

	/** The name of the game feature plugin to change state for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Feature")
	FName GameFeatureName = NAME_None;

	/** The target state to transition the game feature to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Feature")
	EGameFeatureTargetState TargetState = EGameFeatureTargetState::Active;
};