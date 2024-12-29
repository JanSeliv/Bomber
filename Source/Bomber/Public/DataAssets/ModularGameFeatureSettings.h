// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DeveloperSettings.h"
#include "ModularGameFeatureSettings.generated.h"

/**
 * Contains all core tweaks for Modular Game Features (MGF). 
 * Is set up in 'Project Settings' -> 'Game' -> 'Bomber Modular Game Features (MGF)'.
 * The changes are saved in 'DefaultModularGameFeatures.ini' config file.
 * It's not data asset for easier access and better tracking in version control.
 */
UCLASS(Config = "ModularGameFeatures", DefaultConfig, DisplayName = "Bomber Modular Game Features (MGF)")
class BOMBER_API UModularGameFeatureSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Returns this instance. */
	static const UModularGameFeatureSettings& Get() { return *GetDefault<ThisClass>(); }

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
public:
	/** Returns all game features need to be loaded and activated on starting the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE TArray<FName>& GetModularGameFeatures() const { return ModularGameFeaturesInternal; }

protected:
	/** All game features need to be loaded and activated on starting the game, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Modular Game Features", ShowOnlyInnerProperties))
	TArray<FName> ModularGameFeaturesInternal;
};