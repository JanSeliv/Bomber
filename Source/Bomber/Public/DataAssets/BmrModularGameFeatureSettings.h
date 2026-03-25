// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DeveloperSettings.h"

// UE
#include "GameplayTagContainer.h"

#include "BmrModularGameFeatureSettings.generated.h"

/**
 * Contains all core tweaks for Modular Game Features (MGF).
 * Is set up in 'Project Settings' -> 'Game' -> 'Bomber Modular Game Features (MGF)'.
 * The changes are saved in 'DefaultModularGameFeatures.ini' config file.
 * It's not data asset for easier access and better tracking in version control.
 */
UCLASS(Config = "ModularGameFeatures", DefaultConfig, DisplayName = "Bomber Modular Game Features (MGF)")
class BOMBER_API UBmrModularGameFeatureSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Returns this instance. */
	static const UBmrModularGameFeatureSettings& Get() { return *GetDefault<ThisClass>(); }

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
public:
	/** Returns all game features need to be loaded and activated on starting the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE TArray<FName>& GetModularGameFeatures() const { return ModularGameFeatures; }

	/** Returns tag-driven game features map, where key is feature name and value is the set of tags (OR logic: feature loads when ASC has ANY of these tags). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE TMap<FName, FGameplayTagContainer>& GetModularGameFeaturesByTags() const { return ModularGameFeaturesByTags; }

	/** Returns aggregated container of all unique tags from tag-driven features map, lazy-cached. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FGameplayTagContainer& GetAllModularGameFeatureTags() const;

protected:
	/** All game features need to be loaded and activated on starting the game, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties, GetOptions = "MyUtils.ModularGameFeaturePluginUtils.GetAllRegisteredModularGameFeatures"))
	TArray<FName> ModularGameFeatures;

	/** Tag-driven game features: key is feature name, value is the set of gameplay tags (OR logic: feature loads when ASC has ANY of these tags), is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties, GetOptions = "MyUtils.ModularGameFeaturePluginUtils.GetAllRegisteredModularGameFeatures"))
	TMap<FName, FGameplayTagContainer> ModularGameFeaturesByTags;
};