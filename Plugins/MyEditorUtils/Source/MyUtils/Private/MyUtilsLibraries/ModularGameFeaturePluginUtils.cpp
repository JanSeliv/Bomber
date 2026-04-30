// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"

// MyUtils
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/GameFeatureStateChange.h"

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

	const FString NameStr = GameFeatureName.ToString();
	constexpr bool bCheckForActivating = true;

	// Try direct plugin name match first
	if (UGameFeaturesSubsystem::Get().IsGameFeaturePluginActiveByName(NameStr, bCheckForActivating))
	{
		return true;
	}

	// Fallback: module name might differ from plugin name (e.g. "GameFeatureModuleRuntime" vs "GameFeatureModule"), resolve against registered features
	const TArray<FString> RegisteredFeatures = GetAllRegisteredModularGameFeatures();
	for (const FString& FeatureName : RegisteredFeatures)
	{
		if (NameStr.StartsWith(FeatureName))
		{
			return UGameFeaturesSubsystem::Get().IsGameFeaturePluginActiveByName(FeatureName, bCheckForActivating);
		}
	}

	return false;
}

// Returns the built-in initial auto state for a game feature
EGameFeatureTargetState UModularGameFeaturePluginUtils::GetBuiltInInitialFeatureState(FName GameFeatureName)
{
	if (GameFeatureName.IsNone())
	{
		return EGameFeatureTargetState::Installed;
	}

	const UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();

	FString GameFeatureURL;
	GameFeaturesSubsystem.GetPluginURLByName(GameFeatureName.ToString(), /*out*/ GameFeatureURL);
	if (GameFeatureURL.IsEmpty())
	{
		return EGameFeatureTargetState::Installed;
	}

	FGameFeaturePluginDetails PluginDetails;
	GameFeaturesSubsystem.GetGameFeaturePluginDetails(GameFeatureURL, PluginDetails);

	switch (PluginDetails.BuiltInAutoState)
	{
		case EBuiltInAutoState::Invalid:
			return EGameFeatureTargetState::Installed;
		case EBuiltInAutoState::Installed:
			return EGameFeatureTargetState::Installed;
		case EBuiltInAutoState::Registered:
			return EGameFeatureTargetState::Registered;
		case EBuiltInAutoState::Loaded:
			return EGameFeatureTargetState::Loaded;
		case EBuiltInAutoState::Active:
			return EGameFeatureTargetState::Active;
		default:
			return EGameFeatureTargetState::Installed;
	}
}

// Enables or disable all game features
void UModularGameFeaturePluginUtils::SetModularGameFeaturesActive(bool bEnable, const TArray<FName>& GameFeatures)
{
	if (GameFeatures.IsEmpty())
	{
		return;
	}

	TArray<FGameFeatureStateChange> Changes;
	Changes.Reserve(GameFeatures.Num());

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

		EGameFeatureTargetState TargetState;
		if (bEnable)
		{
			TargetState = EGameFeatureTargetState::Active;
		}
		else
		{
			// Do not force full unload, but keep registered if next:
			// - plugin itself has initial state as registered
			// - editor is running, where packages not fully unload by design, attempting to force it would break package load back
			const EGameFeatureTargetState InitialState = GetBuiltInInitialFeatureState(GameFeatureName);
			const bool bInitiallyInstalled = InitialState == EGameFeatureTargetState::Installed;
			const bool bKeepRegistered = !bInitiallyInstalled || UUtilsLibrary::IsEditor();
			TargetState = bKeepRegistered ? EGameFeatureTargetState::Registered : EGameFeatureTargetState::Installed;
		}

		Changes.Emplace(GameFeatureName, TargetState);
	}

	ChangeGameFeatureTargetState(Changes);
}

// Changes target state for game features, batching all requests by state
void UModularGameFeaturePluginUtils::ChangeGameFeatureTargetState(const TArray<FGameFeatureStateChange>& Changes)
{
	if (Changes.IsEmpty())
	{
		return;
	}

	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	static const FGameFeatureProtocolOptions Options = []()
	{
		FGameFeatureProtocolOptions Opts;
		Opts.bBatchProcess = true;
		Opts.bLogErrorOnForcedDependencyCreation = true;
		return Opts;
	}();

	TMap<EGameFeatureTargetState, TArray<FString>> Requests;

	for (const FGameFeatureStateChange& Change : Changes)
	{
		if (Change.GameFeatureName.IsNone())
		{
			continue;
		}

		FString GameFeatureURL;
		GameFeaturesSubsystem.GetPluginURLByName(Change.GameFeatureName.ToString(), /*out*/ GameFeatureURL);
		if (GameFeatureURL.IsEmpty())
		{
			UE_LOG(LogGameFeatures, Log, TEXT("Game feature '%s' is not installed in the project (likely removed or corrupted)"), *Change.GameFeatureName.ToString());
			continue;
		}

		Requests.FindOrAdd(Change.TargetState).Add(GameFeatureURL);
	}

	static const FMultipleGameFeaturePluginsLoaded EmptyDelegate{};

	// Batch multiple requests per specific target state at once
	for (const TPair<EGameFeatureTargetState, TArray<FString>>& It : Requests)
	{
		if (!It.Value.IsEmpty())
		{
			GameFeaturesSubsystem.ChangeGameFeatureTargetState(It.Value, Options, It.Key, EmptyDelegate);
		}
	}
}

// Resets game features to their configured built-in auto state
void UModularGameFeaturePluginUtils::RestoreGameFeatureTargetState(const TArray<FName>& GameFeatures)
{
	if (GameFeatures.IsEmpty())
	{
		return;
	}

	TArray<FGameFeatureStateChange> Changes;
	Changes.Reserve(GameFeatures.Num());

	for (const FName GameFeatureName : GameFeatures)
	{
		if (GameFeatureName.IsNone())
		{
			continue;
		}

		const EGameFeatureTargetState TargetState = GetBuiltInInitialFeatureState(GameFeatureName);
		Changes.Emplace(GameFeatureName, TargetState);
	}

	ChangeGameFeatureTargetState(Changes);
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
FString UModularGameFeaturePluginUtils::GetModuleNameByAsset(const UObject* Asset)
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

// Returns the module name from any object by resolving its class package
FString UModularGameFeaturePluginUtils::GetModuleNameByObject(const UObject* Object)
{
	if (!Object)
	{
		return FString();
	}

	// For C++ objects, extract module name from /Script/ class package
	const FString ClassPackageName = GetNameSafe(Object->GetClass()->GetOutermost());
	static const FString ScriptPrefix = TEXT("/Script/");
	if (ClassPackageName.StartsWith(ScriptPrefix))
	{
		return ClassPackageName.RightChop(ScriptPrefix.Len());
	}

	// For Blueprint objects, extract content root from class package
	FString ModuleName = GetModuleNameByAsset(Object->GetClass());
	ModuleName.RemoveFromStart(TEXT("/"));
	ModuleName.RemoveFromEnd(TEXT("/"));
	return ModuleName;
}

// Returns true if the given object belongs to the same game feature plugin as the specified GameFeatureData
bool UModularGameFeaturePluginUtils::IsInGameFeatureModule(const UObject* Object, const UGameFeatureData* GameFeatureData)
{
	if (!Object || !GameFeatureData)
	{
		return false;
	}

	// Extract plugin name from GameFeatureData content root, e.g. "GameFeatureModule" from "/GameFeatureModule/"
	FString PluginName = GetModuleNameByAsset(GameFeatureData);
	if (PluginName.IsEmpty())
	{
		return false;
	}
	PluginName.RemoveFromStart(TEXT("/"));
	PluginName.RemoveFromEnd(TEXT("/"));

	const FString ModuleName = GetModuleNameByObject(Object);
	return !ModuleName.IsEmpty() && ModuleName.StartsWith(PluginName);
}

// Returns true if the given object belongs to any registered game feature plugin
bool UModularGameFeaturePluginUtils::IsInAnyGameFeatureModule(const UObject* Object)
{
	const FString ModuleName = GetModuleNameByObject(Object);
	if (ModuleName.IsEmpty())
	{
		return false;
	}

	const TArray<FString> RegisteredFeatures = GetAllRegisteredModularGameFeatures();
	for (const FString& FeatureName : RegisteredFeatures)
	{
		if (ModuleName.StartsWith(FeatureName))
		{
			return true;
		}
	}

	return false;
}

// Unloads the specified asset from memory
void UModularGameFeaturePluginUtils::UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences /* = false*/)
{
	if (!ensureMsgf(AssetToUnload, TEXT("ASSERT: [%i] %hs:\n'AssetToUnload' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const FString ModuleMount = GetModuleNameByAsset(AssetToUnload);

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