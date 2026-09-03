// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "UObject/SoftObjectPath.h" // FDirectoryPath

#include "GfpmAction_AddGameplayCuePath.generated.h"

/**
 * Game Feature action that registers own plugin content folders in Gameplay Cue Manager while owning Game Feature Plugin is Active.
 * In editor, folders stay registered from Registered state on, so cue notifies validate and cook without activation.
 * Mirrors purpose of Lyra action of same name.
 */
UCLASS(DisplayName = "GAS Add Gameplay Cue Path")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAction_AddGameplayCuePath : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** Contains folders to register, each resolved against own plugin content root unless already full package path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]", meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> DirectoryPathsToAdd = {FDirectoryPath{TEXT("/AbilitySystem/GameplayCues")}};

#if WITH_EDITOR
	/** Called by editor's Data Validation system when validation runs on this object. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

protected:
	/** Called by Game Features system when owning plugin is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** When owning Game Feature Plugin transitions into Active state. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** When owning Game Feature Plugin transitions out of Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	/** Called by Game Features system when owning plugin is unregistered. */
	virtual void OnGameFeatureUnregistering() override;

#if WITH_EDITORONLY_DATA
	/** When cooker gathers asset bundle data for owning plugin. */
	virtual void AddAdditionalAssetBundleData(struct FAssetBundleData& AssetBundleData) override;
#endif // WITH_EDITORONLY_DATA
#if WITH_EDITOR
	/** Called by editor when any property on this object is changed in details panel. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	/*********************************************************************************************
	 * Internal
	 ********************************************************************************************* */
protected:
	/** Contains own paths currently registered in Gameplay Cue Manager, so removal covers exactly what was added. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	TArray<FString> RegisteredCuePaths;

	/** Registers own content folders in Gameplay Cue Manager, each resolved against own plugin content root. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void RegisterCuePaths();

	/** Removes own registered paths from Gameplay Cue Manager. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void RemoveCuePaths();

	/** Returns own content folders resolved against given plugin content root. */
	UFUNCTION(BlueprintPure, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	TArray<FString> GetOwnCuePaths(const FString& PluginRootPath) const;

	/** Returns own plugin content root, empty string if this action does not belong to any plugin. */
	UFUNCTION(BlueprintPure, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	FString GetPluginRootPath() const;
};
