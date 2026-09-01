// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "UObject/SoftObjectPath.h" // FDirectoryPath

#include "GfpmAction_AddGameplayCuePath.generated.h"

/**
 * Game Feature action that registers own plugin content folders in Gameplay Cue Manager while owning Game Feature Plugin is Active.
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
	/** When owning Game Feature Plugin transitions into Active state. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** When owning Game Feature Plugin transitions out of Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

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

	/** Returns own plugin content root, empty string if this action does not belong to any plugin. */
	UFUNCTION(BlueprintPure, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	FString GetPluginRootPath() const;
};
