// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPath.h" // FDirectoryPath

#include "GfpmAction_AddGameplayCuePath.generated.h"

class AGameplayCueNotify_Actor;

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

	/** Returns own content folders resolved against given plugin content root. */
	UFUNCTION(BlueprintPure, Category = "[Game Feature Plugins Manager]")
	TArray<FString> GetOwnCuePaths(const FString& PluginRootPath) const;

	/** Returns own cue actor classes currently registered in Gameplay Cue Manager, so unload releases exactly what registration added. */
	UFUNCTION(BlueprintPure, Category = "[Game Feature Plugins Manager]")
	TArray<TSubclassOf<AGameplayCueNotify_Actor>> GetOwnCueClasses() const;

	/** Returns the pointer to pool list of preallocated cue actors of Gameplay Cue Manager, nullptr if manager is already gone or no longer exposes it under own name and shape. */
	TArray<struct FPreallocationInfo>* FindCueManagerPoolList() const;

protected:
	/** Called by Game Features system when owning plugin is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** When owning Game Feature Plugin transitions into Active state. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** Called by Game Features system when owning plugin is unloaded. */
	virtual void OnGameFeatureUnloading() override;

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

	/** Destroys actors of given cue classes in every game world, pooled or live, ending live ones first so cue end releases what it spawned. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, AutoCreateRefTerm = "CueClasses"))
	void DestroyOwnCueActors(const TArray<TSubclassOf<AGameplayCueNotify_Actor>>& CueClasses);

	/** Removes given cue classes from Gameplay Cue Manager pool, so no pooled class key pins plugin content past unload. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, AutoCreateRefTerm = "CueClasses"))
	void RemoveOwnCueClassesFromPool(const TArray<TSubclassOf<AGameplayCueNotify_Actor>>& CueClasses);
};
