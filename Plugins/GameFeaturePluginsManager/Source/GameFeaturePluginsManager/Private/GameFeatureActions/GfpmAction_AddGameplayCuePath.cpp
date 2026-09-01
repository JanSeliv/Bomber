// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_AddGameplayCuePath.h"

// UE
#include "AbilitySystemGlobals.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "GameplayCueManager.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"

#if WITH_EDITOR
#include "Internationalization/Text.h"
#include "Misc/DataValidation.h"
#include "UObject/UnrealType.h"
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

// When owning Game Feature Plugin transitions into Active state
void UGfpmAction_AddGameplayCuePath::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);

	RegisterCuePaths();
}

// When owning Game Feature Plugin transitions out of Active state
void UGfpmAction_AddGameplayCuePath::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	RemoveCuePaths();

	Super::OnGameFeatureDeactivating(Context);
}

#if WITH_EDITOR
// Called by editor when any property on this object is changed in details panel
void UGfpmAction_AddGameplayCuePath::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, DirectoryPathsToAdd)
	    && IsGameFeaturePluginActive())
	{
		// Folder list is changed while plugin stays Active, re-register so designer sees own cues resolve without plugin restart
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
	const FString PluginRootPath = GetPluginRootPath();
	if (!ensureMsgf(CueManager, TEXT("ASSERT: [%i] %hs:\n'CueManager' is null!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!PluginRootPath.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'PluginRootPath' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (!RegisteredCuePaths.IsEmpty())
	{
		// Likely previous registration is still live, e.g: designer changed folder list while plugin stays Active
		RemoveCuePaths();
	}

	constexpr bool bMakeRelativeToPluginRoot = false;
	for (const FDirectoryPath& DirectoryIt : DirectoryPathsToAdd)
	{
		FString CuePath = DirectoryIt.Path;
		UGameFeaturesSubsystem::FixPluginPackagePath(/*out*/CuePath, PluginRootPath, bMakeRelativeToPluginRoot);
		RegisteredCuePaths.AddUnique(CuePath);
	}

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

// Returns own plugin content root, empty string if this action does not belong to any plugin
FString UGfpmAction_AddGameplayCuePath::GetPluginRootPath() const
{
	const UGameFeatureData* GameFeatureData = GetGameFeatureData();
	const UPackage* OwnPackage = GameFeatureData ? GameFeatureData->GetOutermost() : nullptr;
	if (!OwnPackage)
	{
		// Likely action is inspected outside any owning Game Feature Data, e.g: class default object in editor action picker
		return FString();
	}

	const FName MountPoint = FPackageName::GetPackageMountPoint(OwnPackage->GetName());
	return MountPoint.IsNone() ? FString() : TEXT("/") + MountPoint.ToString();
}
