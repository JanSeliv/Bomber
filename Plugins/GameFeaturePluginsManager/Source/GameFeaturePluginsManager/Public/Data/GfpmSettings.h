// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DeveloperSettings.h"

// UE
#include "GameplayTagContainer.h"

#include "GfpmSettings.generated.h"

/**
 * Contains all core tweaks for Modular Game Features (MGF).
 * Is set up in 'Project Settings' -> 'Game' -> 'Game Feature Plugins Manager Settings'.
 * The changes are saved in 'DefaultModularGameFeatures.ini' config file.
 * It's not data asset for easier access and better tracking in version control.
 * Features that should always be loaded must set their BuiltInInitialFeatureState to Active in .uplugin instead.
 */
UCLASS(Config = "ModularGameFeatures", DefaultConfig, DisplayName = "Game Feature Plugins Manager Settings")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Returns this instance. */
	static const UGfpmSettings& Get() { return *GetDefault<ThisClass>(); }

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
public:
	/** Returns tag-driven game features map, where key is feature name and value is the set of tags (OR logic: feature loads when ASC has ANY of these tags). Registered features are loaded at runtime when ASC has ANY of specified tags. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	const FORCEINLINE TMap<FName, FGameplayTagContainer>& GetModularGameFeaturesByTags() const { return ModularGameFeaturesByTags; }

	/** Returns aggregated container of all unique tags from tag-driven features map, lazy-cached. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	const FGameplayTagContainer& GetAllModularGameFeatureTags() const;

protected:
	/** Tag-driven game features: key is feature name, value is the set of gameplay tags (OR logic: feature loads when ASC has ANY of these tags), is config property. Registered features are loaded at runtime when ASC has ANY of specified tags. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties, GetOptions = "GameFeaturePluginsManager.GfpmUtils.GetAllRegisteredModularGameFeatures"))
	TMap<FName, FGameplayTagContainer> ModularGameFeaturesByTags;
};
