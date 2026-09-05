// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_AddGameplayCuePath.h"

// GFPM
#include "Data/GfpmScopedWorldContext.h"
#include "GfpmUtils.h"

// UE
#include "AbilitySystemGlobals.h"
#include "CoreGlobals.h"
#include "Engine/World.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "GameplayCueManager.h"
#include "GameplayCueNotify_Actor.h"
#include "GameplayCueSet.h"
#include "GameplayCue_Types.h"
#include "Misc/PackageName.h"
#include "Misc/PathViews.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectHash.h"
#include "UObject/UnrealType.h"

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/AssetBundleData.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/ObjectLibrary.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameplayCueNotify_Static.h"
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
#include "Internationalization/Text.h"
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAction_AddGameplayCuePath)

#if WITH_EDITOR
// Called by editor's Data Validation system when validation runs on this object
EDataValidationResult UGfpmAction_AddGameplayCuePath::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Result = Super::IsDataValid(Context);

	if (DirectoryPathsToAdd.IsEmpty())
	{
		static const FString TmplEmptyList = TEXT("DirectoryPathsToAdd is empty, no Gameplay Cue folder will be registered");
		const FString Formatted = FString::Format(*TmplEmptyList, FStringFormatOrderedArguments{});
		Context.AddWarning(FText::FromString(Formatted));
	}

	for (const FDirectoryPath& DirectoryIt : DirectoryPathsToAdd)
	{
		if (DirectoryIt.Path.IsEmpty())
		{
			static const FString TmplEmptyFolder = TEXT("DirectoryPathsToAdd contains empty folder that can not be registered");
			const FString Formatted = FString::Format(*TmplEmptyFolder, FStringFormatOrderedArguments{});
			Context.AddWarning(FText::FromString(Formatted));
			break;
		}
	}

	return Result;
}
#endif // WITH_EDITOR

// Returns own content folders resolved against given plugin content root
TArray<FString> UGfpmAction_AddGameplayCuePath::GetOwnCuePaths(const FString& PluginRootPath) const
{
	TArray<FString> CuePaths;
	constexpr bool bMakeRelativeToPluginRoot = false;
	for (const FDirectoryPath& DirectoryIt : DirectoryPathsToAdd)
	{
		FString CuePath = DirectoryIt.Path;
		UGameFeaturesSubsystem::FixPluginPackagePath(/*out*/ CuePath, PluginRootPath, bMakeRelativeToPluginRoot);
		CuePaths.AddUnique(CuePath);
	}
	return CuePaths;
}

// Returns own cue actor classes currently registered in Gameplay Cue Manager, so unload releases exactly what registration added
TArray<TSubclassOf<AGameplayCueNotify_Actor>> UGfpmAction_AddGameplayCuePath::GetOwnCueClasses() const
{
	TArray<TSubclassOf<AGameplayCueNotify_Actor>> OwnCueClasses;
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	const UGameplayCueSet* CueSet = CueManager ? CueManager->GetRuntimeCueSet() : nullptr;
	if (RegisteredCuePaths.IsEmpty()
	    || !CueSet)
	{
		// Likely nothing is registered yet, or Gameplay Cue Manager is already gone during engine teardown
		return OwnCueClasses;
	}

	for (const FGameplayCueNotifyData& CueDataIt : CueSet->GameplayCueData)
	{
		UClass* CueClass = CueDataIt.LoadedGameplayCueClass;
		if (!CueClass
		    || !CueClass->IsChildOf<AGameplayCueNotify_Actor>())
		{
			// Likely class not loaded yet, or static cue that has no actor to release
			continue;
		}

		const FString CueFolder = FPackageName::GetLongPackagePath(CueDataIt.GameplayCueNotifyObj.GetLongPackageName());
		const bool bIsOwnFolder = RegisteredCuePaths.ContainsByPredicate([&CueFolder](const FString& RegisteredPathIt)
		{
			return FPathViews::IsParentPathOf(RegisteredPathIt, CueFolder);
		});
		if (bIsOwnFolder)
		{
			OwnCueClasses.AddUnique(CueClass);
		}
	}
	return OwnCueClasses;
}

// Returns the pointer to pool list of preallocated cue actors of Gameplay Cue Manager, nullptr if manager is already gone or no longer exposes it under own name and shape
TArray<FPreallocationInfo>* UGfpmAction_AddGameplayCuePath::FindCueManagerPoolList() const
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	if (!CueManager)
	{
		// Likely Gameplay Cue Manager is already gone during engine teardown
		return nullptr;
	}

	// Pool list is protected member of engine manager, but its extremely important to cleanup (engine issue itself), so reach out it by reflection
	static const FName PoolListName = TEXT("PreallocationInfoList_Internal");
	const FArrayProperty* PoolListProperty = CastField<FArrayProperty>(UGameplayCueManager::StaticClass()->FindPropertyByName(PoolListName));
	const FStructProperty* PoolInfoProperty = PoolListProperty ? CastField<FStructProperty>(PoolListProperty->Inner) : nullptr;
	const bool bIsPoolList = PoolInfoProperty && PoolInfoProperty->Struct == FPreallocationInfo::StaticStruct();
	if (!ensureMsgf(bIsPoolList, TEXT("ASSERT: [%i] %hs:\n'%s' no longer resolves to pool list of Gameplay Cue Manager, engine likely renamed it, so pooled cue classes outlive plugin unload!"), __LINE__, __FUNCTION__, *PoolListName.ToString()))
	{
		return nullptr;
	}
	return PoolListProperty->ContainerPtrToValuePtr<TArray<FPreallocationInfo>>(CueManager);
}

// Called by Game Features system when owning plugin is registered
void UGfpmAction_AddGameplayCuePath::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	if (GIsEditor)
	{
		// In editor, preload early, so Cues can be validated and cooked
		RegisterCuePaths();
	}
}

// When owning Game Feature Plugin transitions into Active state
void UGfpmAction_AddGameplayCuePath::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);

	RegisterCuePaths();
}

// Called by Game Features system when owning plugin is unloaded
void UGfpmAction_AddGameplayCuePath::OnGameFeatureUnloading()
{
	const TArray<TSubclassOf<AGameplayCueNotify_Actor>> OwnCueClasses = GetOwnCueClasses();
	DestroyOwnCueActors(OwnCueClasses);
	RemoveOwnCueClassesFromPool(OwnCueClasses);

	if (!GIsEditor)
	{
		// Only outside editor itself unload early, e.g: when stop PIE Cues must remain registered
		RemoveCuePaths();
	}

	Super::OnGameFeatureUnloading();
}

// Called by Game Features system when owning plugin is unregistered
void UGfpmAction_AddGameplayCuePath::OnGameFeatureUnregistering()
{
	RemoveCuePaths();

	Super::OnGameFeatureUnregistering();
}

#if WITH_EDITORONLY_DATA
// When cooker gathers asset bundle data for owning plugin
void UGfpmAction_AddGameplayCuePath::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	const FString PluginRootPath = UGfpmUtils::GetPluginRootPathByAsset(GetGameFeatureData());
	if (!UAssetManager::IsInitialized()
	    || PluginRootPath.IsEmpty())
	{
		return;
	}

	constexpr bool bHasBlueprintClasses = true;
	constexpr bool bUseWeakReferences = true;
	const TArray<FString> CuePaths = GetOwnCuePaths(PluginRootPath);
	const TArray NotifyClasses = {UGameplayCueNotify_Static::StaticClass(), AGameplayCueNotify_Actor::StaticClass()};
	for (UClass* NotifyClassIt : NotifyClasses)
	{
		UObjectLibrary* NotifyLibrary = UObjectLibrary::CreateLibrary(NotifyClassIt, bHasBlueprintClasses, bUseWeakReferences);
		NotifyLibrary->LoadBlueprintAssetDataFromPaths(CuePaths);

		TArray<FAssetData> NotifyAssets;
		NotifyLibrary->GetAssetDataList(/*out*/ NotifyAssets);
		for (const FAssetData& NotifyAssetIt : NotifyAssets)
		{
			// Cue notify resolves by tag only, so bundle keeps it referenced for cook and preloads it with owning plugin
			const FTopLevelAssetPath NotifyAssetPath = NotifyAssetIt.GetSoftObjectPath().GetAssetPath();
			AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, NotifyAssetPath);
			AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, NotifyAssetPath);
		}
	}
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
// Called by editor when any property on this object is changed in details panel
void UGfpmAction_AddGameplayCuePath::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, DirectoryPathsToAdd)
	    && IsGameFeaturePluginRegistered())
	{
		// Folder list is changed while plugin stays Registered, re-register so designer sees own cues resolve without plugin restart
		RegisterCuePaths();
	}
}
#endif // WITH_EDITOR

/*********************************************************************************************
 * Internal
 ********************************************************************************************* */

// Registers own content folders in Gameplay Cue Manager, each resolved against own plugin content root
void UGfpmAction_AddGameplayCuePath::RegisterCuePaths()
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	const FString PluginRootPath = UGfpmUtils::GetPluginRootPathByAsset(GetGameFeatureData());
	if (!ensureMsgf(CueManager, TEXT("ASSERT: [%i] %hs:\n'CueManager' is null!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!PluginRootPath.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'PluginRootPath' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const TArray<FString> CuePaths = GetOwnCuePaths(PluginRootPath);
	if (CuePaths == RegisteredCuePaths)
	{
		// Likely same folders are registered already, e.g: editor registers on Registered state and again on activation
		return;
	}

	if (!RegisteredCuePaths.IsEmpty())
	{
		// Likely previous registration is still live, e.g: designer changed folder list while plugin stays Registered
		RemoveCuePaths();
	}

	RegisteredCuePaths = CuePaths;
	if (RegisteredCuePaths.IsEmpty())
	{
		// Likely no folder is set, so nothing to register
		return;
	}

	constexpr bool bShouldRescanCueAssets = false;
	for (const FString& CuePathIt : RegisteredCuePaths)
	{
		CueManager->AddGameplayCueNotifyPath(CuePathIt, bShouldRescanCueAssets);
	}

	// Per-path rescan is skipped above, so object library is rebuilt once for whole batch
	CueManager->InitializeRuntimeObjectLibrary();
}

// Removes own registered paths from Gameplay Cue Manager
void UGfpmAction_AddGameplayCuePath::RemoveCuePaths()
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	if (RegisteredCuePaths.IsEmpty()
	    || !CueManager)
	{
		// Likely nothing is registered yet, or Gameplay Cue Manager is already gone during engine teardown
		RegisteredCuePaths.Empty();
		return;
	}

	constexpr bool bShouldRescanCueAssets = false;
	int32 RemovedNum = 0;
	for (const FString& CuePathIt : RegisteredCuePaths)
	{
		RemovedNum += CueManager->RemoveGameplayCueNotifyPath(CuePathIt, bShouldRescanCueAssets);
	}

	RegisteredCuePaths.Empty();

	if (RemovedNum > 0)
	{
		CueManager->InitializeRuntimeObjectLibrary();
	}
}

// Destroys actors of given cue classes in every game world, pooled or live, ending live ones first so cue end releases what it spawned
void UGfpmAction_AddGameplayCuePath::DestroyOwnCueActors(const TArray<TSubclassOf<AGameplayCueNotify_Actor>>& CueClasses)
{
	constexpr bool bIncludeDerivedClasses = false;
	for (const TSubclassOf<AGameplayCueNotify_Actor>& CueClassIt : CueClasses)
	{
		TArray<UObject*> OutCueObjects;
		GetObjectsOfClass(CueClassIt, OutCueObjects, bIncludeDerivedClasses, RF_ClassDefaultObject | RF_ArchetypeObject, EInternalObjectFlags::Garbage);
		for (UObject* CueObjectIt : OutCueObjects)
		{
			AGameplayCueNotify_Actor& CueActorRef = *CastChecked<AGameplayCueNotify_Actor>(CueObjectIt);
			UWorld* World = CueActorRef.GetWorld();
			if (!World
			    || !World->IsGameWorld())
			{
				// Likely actor of editor or preview world that never pools cues
				continue;
			}

			// Own cue releases what it spawned inside that world, so globals point at it while cue end runs
			FGfpmScopedWorldContext WorldContextGuard(World);
			if (!CueActorRef.bInRecycleQueue)
			{
				// Live actor still represents running cue, so it is ended first and releases own effects before it leaves
				CueActorRef.K2_EndGameplayCue();
			}

			CueActorRef.Destroy();
		}
	}
}

// Removes given cue classes from Gameplay Cue Manager pool, so no pooled class key pins plugin content past unload
void UGfpmAction_AddGameplayCuePath::RemoveOwnCueClassesFromPool(const TArray<TSubclassOf<AGameplayCueNotify_Actor>>& CueClasses)
{
	TArray<FPreallocationInfo>* PoolList = FindCueManagerPoolList();
	if (!PoolList)
	{
		// Likely Gameplay Cue Manager is already gone during engine teardown, or engine no longer exposes own pool
		return;
	}

	for (FPreallocationInfo& PoolInfoIt : *PoolList)
	{
		for (const TSubclassOf<AGameplayCueNotify_Actor>& CueClassIt : CueClasses)
		{
			PoolInfoIt.PreallocatedInstances.Remove(CueClassIt);
		}
	}
}
