// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EditorSubsystem.h"

#include "GfpmPackageEditorSubsystem.generated.h"

/**
 * Packages Game Feature Plugins as separate mods from running editor and installs them into chosen build.
 */
UCLASS(Config = "GameFeaturePluginsManager", DefaultConfig, DisplayName = "Game Feature Plugins Package Editor Subsystem")
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmPackageEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/** Packages given plugin as separate mod and installs it into build at given path.
	 * @param GameFeatureName Plugin to package.
	 * @param BuildPath Build to install into, or any folder for separate distribution. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]", meta = (AutoCreateRefTerm = "BuildPath"))
	void PackageGameFeatureIntoBuild(FName GameFeatureName, const FString& BuildPath);

	/** Packages every registered plugin as separate mod and installs them into build at given path.
	 * @param BuildPath Build to install into, or any folder for separate distribution. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]", meta = (AutoCreateRefTerm = "BuildPath"))
	void PackageAllGameFeaturesIntoBuild(const FString& BuildPath);

	/** Prompts developer for target build folder then packages given plugin into it. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]")
	void PromptPackageGameFeature(FName GameFeatureName);

	/** Prompts developer for target build folder then packages every registered plugin into it. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]")
	void PromptPackageAllGameFeatures();

protected:
	/** Release version mod is cooked against, must match build's release. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	FString ProjectReleaseVersion;

	/** Static cook arguments shared by every mod cook, override per project pipeline. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	FString ModCookArguments;

	/** Cook arguments full project build uses to package without game features, override per project pipeline. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	FString ProjectCookArguments;

	/** Extra cooker options appended to engine additional cooker args, override per project pipeline. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	FString AdditionalCookerOptions;

private:
	/** Opens desktop folder picker with given dialog title, empty when developer cancels. Not exposed: editor desktop dialog. */
	FString PromptBuildDir(const FString& DialogTitle) const;

	/** Launches out-of-process cook of one plugin into build, runs continuation after install. Not exposed: TFunction continuation. */
	void LaunchModPackage(FName GameFeatureName, const FString& BuildPath, TFunction<void()> OnComplete);

	/** Packages modular project without game features into build path, runs continuation after it succeeds. Not exposed: TFunction continuation. */
	void LaunchProjectPackage(const FString& BuildPath, TFunction<void()> OnComplete);

	/** Packages queue head into build then recurses on completion, so plugins package one after another. Not exposed: private recursive implementation, covered by public BP wrapper. */
	void PackageModQueue(const TArray<FName>& Queue, const FString& BuildPath);

	/** Installs cooked mod into build then drives queue continuation, runs on game thread after package task completes. Not exposed: TFunction continuation. */
	void HandlePackageFinished(const FString& Result, FName GameFeatureName, const FString& ArchiveDir, const FString& TargetPaksDir, const FString& CookedPlatform, const TFunction<void()>& OnComplete);

	/** Drives queue continuation once project package succeeds, runs on game thread after package task completes. Not exposed: TFunction continuation. */
	void HandleProjectPackageFinished(const FString& Result, const TFunction<void()>& OnComplete);
};
