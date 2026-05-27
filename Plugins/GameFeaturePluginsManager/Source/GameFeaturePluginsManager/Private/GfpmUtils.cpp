// Copyright (c) Yevhenii Selivanov

#include "GfpmUtils.h"

// GFPM
#include "Data/GfpmStateChange.h"

// UE
#include "Engine/World.h"
#include "GameFeatureData.h"
#include "GameFeatureTypes.h"
#include "GameFeaturesSubsystem.h"
#include "UObject/Package.h"
#include "UnrealEngine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmUtils)

// Returns true if specified game feature plugin is currently active
bool UGfpmUtils::IsGameFeaturePluginActive(FName GameFeatureName, bool bCheckForPending /*= false*/)
{
	const FString GameFeatureURL = FindPluginURLByName(GameFeatureName);
	if (GameFeatureURL.IsEmpty())
	{
		return false;
	}

	const EGameFeaturePluginState CurrentState = UGameFeaturesSubsystem::Get().GetPluginState(GameFeatureURL);

	// Stable active states
	if (CurrentState == EGameFeaturePluginState::Active
	    || CurrentState == EGameFeaturePluginState::Activating)
	{
		return true;
	}

	if (!bCheckForPending)
	{
		return false;
	}

	// Earlier transition states moving toward Active
	return CurrentState == EGameFeaturePluginState::Loading
	       || CurrentState == EGameFeaturePluginState::Loaded
	       || CurrentState == EGameFeaturePluginState::ActivatingDependencies;
}

// Returns true if specified game feature plugin is currently inactive
bool UGfpmUtils::IsGameFeaturePluginInactive(FName GameFeatureName, bool bCheckForPending /*= false*/)
{
	const FString GameFeatureURL = FindPluginURLByName(GameFeatureName);
	if (GameFeatureURL.IsEmpty())
	{
		return false;
	}

	const EGameFeaturePluginState CurrentState = UGameFeaturesSubsystem::Get().GetPluginState(GameFeatureURL);

	// Stable inactive states
	if (CurrentState == EGameFeaturePluginState::Installed
	    || CurrentState == EGameFeaturePluginState::Registered)
	{
		return true;
	}

	if (!bCheckForPending)
	{
		return false;
	}

	// Transition states moving toward inactive
	return CurrentState == EGameFeaturePluginState::Unregistering
	       || CurrentState == EGameFeaturePluginState::Unloading;
}

// Resolves URL for a registered game feature plugin by name, with module-suffix fallback
FString UGfpmUtils::FindPluginURLByName(FName GameFeatureName)
{
	if (GameFeatureName.IsNone())
	{
		return FString();
	}

	const FString NameStr = GameFeatureName.ToString();
	const UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();

	// Direct plugin name match first
	FString GameFeatureURL;
	GameFeaturesSubsystem.GetPluginURLByName(NameStr, /*out*/ GameFeatureURL);
	if (!GameFeatureURL.IsEmpty())
	{
		return GameFeatureURL;
	}

	// Fallback: runtime module name may differ from plugin name (e.g. "GameFeatureModuleRuntime" vs "GameFeatureModule"), probe registered features for a prefix match
	const TArray<FString> RegisteredFeatures = GetAllRegisteredGameFeaturePlugins();
	for (const FString& FeatureName : RegisteredFeatures)
	{
		if (NameStr.StartsWith(FeatureName))
		{
			GameFeaturesSubsystem.GetPluginURLByName(FeatureName, /*out*/ GameFeatureURL);
			return GameFeatureURL;
		}
	}

	return FString();
}

// Returns the built-in initial auto state for a game feature
EGameFeatureTargetState UGfpmUtils::GetBuiltInInitialFeatureState(FName GameFeatureName)
{
	if (GameFeatureName.IsNone())
	{
		return EGameFeatureTargetState::Installed;
	}

	const FString GameFeatureURL = FindPluginURLByName(GameFeatureName);
	if (GameFeatureURL.IsEmpty())
	{
		return EGameFeatureTargetState::Installed;
	}

	FGameFeaturePluginDetails PluginDetails;
	UGameFeaturesSubsystem::Get().GetGameFeaturePluginDetails(GameFeatureURL, PluginDetails);

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
void UGfpmUtils::SetGameFeaturePluginsActive(bool bEnable, const TArray<FName>& GameFeatures)
{
	if (GameFeatures.IsEmpty())
	{
		return;
	}

	TArray<FGfpmStateChange> Changes;
	Changes.Reserve(GameFeatures.Num());

	for (const FName GameFeatureName : GameFeatures)
	{
		if (GameFeatureName.IsNone())
		{
			continue;
		}

		constexpr bool bCheckForPending = true;
		const bool bSkip = bEnable ? IsGameFeaturePluginActive(GameFeatureName, bCheckForPending) : IsGameFeaturePluginInactive(GameFeatureName, bCheckForPending);
		if (bSkip)
		{
			// GFP is already at or transitioning toward specified state, is likely caused by calling multiple times in this or following frames while request is still processing
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
#if WITH_EDITOR
			const bool bIsEditor = GIsEditor && GWorld && GWorld->IsEditorWorld();
#else
			const bool bIsEditor = false;
#endif // WITH_EDITOR
			const EGameFeatureTargetState InitialState = GetBuiltInInitialFeatureState(GameFeatureName);
			const bool bInitiallyInstalled = InitialState == EGameFeatureTargetState::Installed;
			const bool bKeepRegistered = !bInitiallyInstalled || bIsEditor;
			TargetState = bKeepRegistered ? EGameFeatureTargetState::Registered : EGameFeatureTargetState::Installed;
		}

		Changes.Emplace(GameFeatureName, TargetState);
	}

	ChangeGameFeatureTargetState(Changes);
}

// Changes target state for game features, batching all requests by state
void UGfpmUtils::ChangeGameFeatureTargetState(const TArray<FGfpmStateChange>& Changes)
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

	for (const FGfpmStateChange& Change : Changes)
	{
		if (Change.GameFeatureName.IsNone())
		{
			continue;
		}

		const FString GameFeatureURL = FindPluginURLByName(Change.GameFeatureName);
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
void UGfpmUtils::RestoreGameFeatureTargetState(const TArray<FName>& GameFeatures)
{
	if (GameFeatures.IsEmpty())
	{
		return;
	}

	TArray<FGfpmStateChange> Changes;
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

// Returns names of all registered game feature plugins
TArray<FString> UGfpmUtils::GetAllRegisteredGameFeaturePlugins()
{
	TArray<FString> FeatureNames;
	UGameFeaturesSubsystem::Get().ForEachGameFeature([&FeatureNames](FGameFeatureInfo&& Info)
	{
		FeatureNames.Emplace(MoveTemp(Info.Name));
	});
	return FeatureNames;
}

// Returns the content module name from the specified asset package
FName UGfpmUtils::GetModuleNameByAsset(const UObject* Asset)
{
	if (!Asset)
	{
		// Null asset has no package to resolve
		return NAME_None;
	}

	const FString OriginalPackageName = GetNameSafe(Asset->GetOutermost());
	const int32 SecondSlashIdx = OriginalPackageName.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 1);
	return SecondSlashIdx != INDEX_NONE ? FName(*OriginalPackageName.Mid(1, SecondSlashIdx - 1)) : NAME_None;
}

// Returns the module name from any object by resolving its class package
FName UGfpmUtils::GetModuleNameByObject(const UObject* Object)
{
	if (!Object)
	{
		return NAME_None;
	}

	// For C++ objects, extract module name from /Script/ class package
	const FString ClassPackageName = GetNameSafe(Object->GetClass()->GetOutermost());
	static const FString ScriptPrefix = TEXT("/Script/");
	if (ClassPackageName.StartsWith(ScriptPrefix))
	{
		return FName(*ClassPackageName.RightChop(ScriptPrefix.Len()));
	}

	// For Blueprint objects, extract content root from class package
	return GetModuleNameByAsset(Object->GetClass());
}

// Returns true if the given object belongs to the same game feature plugin as the specified GameFeatureData
bool UGfpmUtils::IsInGameFeatureModule(const UObject* Object, const UGameFeatureData* GameFeatureData)
{
	if (!Object || !GameFeatureData)
	{
		return false;
	}

	const FName PluginName = GetModuleNameByAsset(GameFeatureData);
	if (PluginName.IsNone())
	{
		// GameFeatureData has no resolvable plugin name
		return false;
	}

	// Resolve by object first
	FName ModuleName = GetModuleNameByObject(Object);
	if (ModuleName.ToString().StartsWith(PluginName.ToString()))
	{
		return true;
	}

	// Resolve by asset name
	ModuleName = GetModuleNameByAsset(Object);
	return ModuleName.ToString().StartsWith(PluginName.ToString());
}

// Returns true if the given object belongs to any registered game feature plugin
bool UGfpmUtils::IsInAnyGameFeatureModule(const UObject* Object)
{
	const FString ModuleName = GetModuleNameByObject(Object).ToString();
	if (ModuleName.IsEmpty())
	{
		return false;
	}

	const TArray<FString> RegisteredFeatures = GetAllRegisteredGameFeaturePlugins();
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
void UGfpmUtils::UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences /* = false*/)
{
	if (!ensureMsgf(AssetToUnload, TEXT("ASSERT: [%i] %hs:\n'AssetToUnload' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const FString ModuleMount = FString::Printf(TEXT("/%s/"), *GetModuleNameByAsset(AssetToUnload).ToString());

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
