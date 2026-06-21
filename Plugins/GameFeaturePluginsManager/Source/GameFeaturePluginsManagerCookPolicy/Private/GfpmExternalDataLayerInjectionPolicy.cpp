// Copyright (c) Yevhenii Selivanov

#include "GfpmExternalDataLayerInjectionPolicy.h"

// UE
#include "CoreGlobals.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "WorldPartition/DataLayer/ExternalDataLayerAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmExternalDataLayerInjectionPolicy)

// Installs injection policy into engine config at early loading phase, before External Data Layer subsystem binds it, so plugin needs no project-side config
class FGameFeaturePluginsManagerCookPolicyModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FString ConfiguredInjectionPolicy;
		GConfig->GetString(TEXT("/Script/Engine.ExternalDataLayerEngineSubsystem"), TEXT("InjectionPolicyClass"), ConfiguredInjectionPolicy, GEngineIni);
		if (ConfiguredInjectionPolicy.IsEmpty())
		{
			static const FString GfpmInjectionPolicyPath = TEXT("/Script/GameFeaturePluginsManagerCookPolicy.GfpmExternalDataLayerInjectionPolicy");
			GConfig->SetString(TEXT("/Script/Engine.ExternalDataLayerEngineSubsystem"), TEXT("InjectionPolicyClass"), *GfpmInjectionPolicyPath, GEngineIni);
		}
	}
};

IMPLEMENT_MODULE(FGameFeaturePluginsManagerCookPolicyModule, GameFeaturePluginsManagerCookPolicy)

#if WITH_EDITOR
namespace GfpmCookPolicyInternal
{
	// Reads opt-in project-exclusion flag from asset manager defaults via reflection, so early module needs no link dependency on it
	static bool ShouldExcludeGameFeaturesFromProjectCook()
	{
		if (FParse::Param(FCommandLine::Get(), TEXT("GfpmExcludeGameFeatures")))
		{
			// Project package cook forces exclusion via param, ignores project config
			return true;
		}

		const UClass* AssetManagerClass = FindObject<UClass>(nullptr, TEXT("/Script/GameFeaturePluginsManager.GfpmAssetManager"));
		if (!AssetManagerClass)
		{
			// Game Feature Plugins Manager runtime absent, nothing opts in
			return false;
		}

		const FBoolProperty* ExcludeProperty = CastField<FBoolProperty>(AssetManagerClass->FindPropertyByName(TEXT("bExcludeGameFeaturesFromProjectCook")));
		return ExcludeProperty && ExcludeProperty->GetPropertyValue_InContainer(AssetManagerClass->GetDefaultObject());
	}

	// Content-root the External Data Layer mounts under, e.g. plugin name, empty when none
	static FString GetMountRoot(const UExternalDataLayerAsset* InExternalDataLayerAsset)
	{
		const UPackage* ExternalDataLayerPackage = InExternalDataLayerAsset ? InExternalDataLayerAsset->GetPackage() : nullptr;
		if (!ExternalDataLayerPackage)
		{
			return FString();
		}

		FString MountRoot = ExternalDataLayerPackage->GetName();
		MountRoot.RemoveFromStart(TEXT("/"));
		int32 SlashIndex = INDEX_NONE;
		if (MountRoot.FindChar(TEXT('/'), SlashIndex))
		{
			MountRoot.LeftInline(SlashIndex);
		}
		return MountRoot;
	}
}

// When cook decides whether given External Data Layer injects into its host world
bool UGfpmExternalDataLayerInjectionPolicy::CanInject(const UWorld* InWorld, const UExternalDataLayerAsset* InExternalDataLayerAsset, const UObject* InClient, FText* OutFailureReason) const
{
	if (!Super::CanInject(InWorld, InExternalDataLayerAsset, InClient, OutFailureReason))
	{
		// Base policy already rejects, keep its decision
		return false;
	}

	if (!IsRunningCookCommandlet())
	{
		// Editor and runtime injection behave as base policy, only cook changes injection
		return true;
	}

	const FString MountRoot = GfpmCookPolicyInternal::GetMountRoot(InExternalDataLayerAsset);
	const bool bIsPluginExternalDataLayer = !MountRoot.IsEmpty() && MountRoot != TEXT("Game") && MountRoot != TEXT("Engine");
	if (!bIsPluginExternalDataLayer)
	{
		// Base game External Data Layer, never affected
		return true;
	}

	FString DLCName;
	if (FParse::Value(FCommandLine::Get(), TEXT("DLCName="), DLCName) && !DLCName.IsEmpty())
	{
		// Mod cook injects only this mod's own External Data Layer, others would regenerate their cells into this mod
		return MountRoot == DLCName;
	}

	if (!GfpmCookPolicyInternal::ShouldExcludeGameFeaturesFromProjectCook())
	{
		// Project cook and project ships game features inline, keep injection
		return true;
	}

	// Project cook leaves game feature plugins out, so its host world generates no cells for them
	if (OutFailureReason)
	{
		*OutFailureReason = FText::FromString(FString::Printf(TEXT("External Data Layer '%s' left out of project cook, ships in its game feature plugin mod"), *InExternalDataLayerAsset->GetName()));
	}
	return false;
}
#endif // WITH_EDITOR
