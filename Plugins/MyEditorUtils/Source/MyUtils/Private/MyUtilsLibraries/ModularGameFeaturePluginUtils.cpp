// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"

// MyUtils
#include "MyUtilsLibraries/UtilsLibrary.h"

// Unreal
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "UObject/Package.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameFeaturePluginUtils)

// Returns true if specified Modular Game Feature plugin is currently active
bool UModularGameFeaturePluginUtils::IsModularGameFeatureActive(FName GameFeatureName)
{
	if (GameFeatureName.IsNone())
	{
		return false;
	}

	constexpr bool bCheckForActivating = true;
	return UGameFeaturesSubsystem::Get().IsGameFeaturePluginActiveByName(GameFeatureName.ToString(), bCheckForActivating);
}

// Enables or disable all game features
void UModularGameFeaturePluginUtils::SetModularGameFeaturesActive(bool bEnable, const TArray<FName>& GameFeatures)
{
	if (GameFeatures.IsEmpty())
	{
		return;
	}

	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	for (const FName GameFeatureName : GameFeatures)
	{
		if (GameFeatureName.IsNone())
		{
			continue;
		}

		const bool bAlreadyActive = IsModularGameFeatureActive(GameFeatureName);
		if (bAlreadyActive == bEnable)
		{
			continue;
		}

		FString GameFeatureURL;
		GameFeaturesSubsystem.GetPluginURLByName(GameFeatureName.ToString(), /*out*/ GameFeatureURL);
		if (!ensureMsgf(!GameFeatureURL.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'%s' game feature state can not be changed!"), __LINE__, __FUNCTION__, *GameFeatureName.ToString()))
		{
			continue;
		}

		static const FGameFeaturePluginLoadComplete EmptyCallback{};
		if (bEnable)
		{
			GameFeaturesSubsystem.LoadAndActivateGameFeaturePlugin(GameFeatureURL, EmptyCallback);
		}
		else
		{
			GameFeaturesSubsystem.UnloadGameFeaturePlugin(GameFeatureURL, EmptyCallback, UUtilsLibrary::IsEditor());
		}
	}
}

// Returns names of all registered Modular Game Feature plugins
TArray<FString> UModularGameFeaturePluginUtils::GetAllRegisteredModularGameFeatures()
{
	TArray<FString> FeatureNames;
	UGameFeaturesSubsystem::Get().ForEachGameFeature([&FeatureNames](FGameFeatureInfo&& Info)
	{
		FeatureNames.Emplace(MoveTemp(Info.Name));
	});
	return FeatureNames;
}

// Returns the module name from the specified asset, if it is part of a game feature
FString UModularGameFeaturePluginUtils::GetModuleNameFromAsset(const UObject* Asset)
{
	FString GameFeatureName;
	if (!Asset)
	{
		return GameFeatureName;
	}

	const FString OriginalPackageName = GetNameSafe(Asset->GetOutermost());
	const int32 SecondSlashIdx = OriginalPackageName.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 1);
	return SecondSlashIdx != INDEX_NONE ? OriginalPackageName.Left(SecondSlashIdx + 1) : FString();
}

// Returns true if the given object belongs to the same game feature plugin as the specified GameFeatureData
bool UModularGameFeaturePluginUtils::IsInGameFeatureModule(const UObject* Object, const UGameFeatureData* GameFeatureData)
{
	if (!Object || !GameFeatureData)
	{
		return false;
	}

	// Get content root from GameFeatureData, e.g. "/GameFeatureModule/"
	const FString PluginContentRoot = GetModuleNameFromAsset(GameFeatureData);
	if (PluginContentRoot.IsEmpty())
	{
		return false;
	}

	// Content path comparison works for Blueprint assets, Data Assets in the same content folder
	const FString ObjectContentRoot = GetModuleNameFromAsset(Object);
	if (ObjectContentRoot == PluginContentRoot)
	{
		return true;
	}

	// For C++ runtime objects (subsystems, components), resolve from the class module package
	const FString ClassPackageName = GetNameSafe(Object->GetClass()->GetOutermost());
	static const FString ScriptPrefix = TEXT("/Script/");
	if (!ClassPackageName.StartsWith(ScriptPrefix))
	{
		return false;
	}

	// Extract C++ module name, e.g. "GameFeatureModuleRuntime" from "/Script/GameFeatureModuleRuntime"
	const FString CppModuleName = ClassPackageName.RightChop(ScriptPrefix.Len());

	// Extract plugin name from content root, e.g. "GameFeatureModule" from "/GameFeatureModule/"
	FString PluginName = PluginContentRoot;
	PluginName.RemoveFromStart(TEXT("/"));
	PluginName.RemoveFromEnd(TEXT("/"));

	// C++ module name starts with the plugin name (e.g. "GameFeatureModuleRuntime" starts with "GameFeatureModule")
	return CppModuleName.StartsWith(PluginName);
}

// Unloads the specified asset from memory
void UModularGameFeaturePluginUtils::UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences /* = false*/)
{
	if (!ensureMsgf(AssetToUnload, TEXT("ASSERT: [%i] %hs:\n'AssetToUnload' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const FString ModuleMount = GetModuleNameFromAsset(AssetToUnload);

	AssetToUnload->ClearFlags(RF_Standalone);
	AssetToUnload->Rename(nullptr, GetTransientPackage(), REN_ForceNoResetLoaders | REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional);

	if (bUnloadReferences)
	{
		TArray<UObject*> ReferencedObjects;
		constexpr bool bInRequireDirectOuter = false;
		constexpr bool bInShouldIgnoreArchetype = true;
		constexpr bool bInSerializeRecursively = false;
		constexpr bool bInShouldIgnoreTransient = true;
		FReferenceFinder ObjectFinder(ReferencedObjects, nullptr, bInRequireDirectOuter, bInShouldIgnoreArchetype, bInSerializeRecursively, bInShouldIgnoreTransient);
		ObjectFinder.FindReferences(AssetToUnload);

		for (UObject* ReferencedObject : ReferencedObjects)
		{
			if (ReferencedObject
			    && GetNameSafe(ReferencedObject->GetOutermost()).StartsWith(ModuleMount))
			{
				constexpr bool bRecursiveUnload = false;
				UnloadAsset(ReferencedObject, bRecursiveUnload);
			}
		}
	}
}