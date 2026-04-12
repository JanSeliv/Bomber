// Copyright (c) Yevhenii Selivanov

#include "Subsystems/ModularGameFeaturePluginSubsystem.h"

// UE
#include "GameFeaturesSubsystem.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameFeaturePluginSubsystem)

/*********************************************************************************************
 * INTERNAL: do not override these in child classes, use OnGameFeatureInitialize/OnGameFeatureDeinitialize instead!
 ********************************************************************************************* */

void UModularGameFeaturePluginSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Tick is disabled by default, call SetTickableTickType to enable/disable it at runtime
	SetTickableTickType(ETickableTickType::Never);
}

// Registers as game feature observer and triggers initial activation if the plugin is already active
void UModularGameFeaturePluginSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);

	const FName ModuleName = FName(*UModularGameFeaturePluginUtils::GetModuleNameByObject(this));
	if (UModularGameFeaturePluginUtils::IsModularGameFeatureActive(ModuleName))
	{
		OnGameFeatureInitialize();
	}
}

// Triggers deactivation if the plugin is still active and unregisters from game feature observer
void UModularGameFeaturePluginSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	const FName ModuleName = FName(*UModularGameFeaturePluginUtils::GetModuleNameByObject(this));
	if (UModularGameFeaturePluginUtils::IsModularGameFeatureActive(ModuleName))
	{
		OnGameFeatureDeinitialize();
	}

	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	Super::OnWorldEndPlay(InWorld);
}

// Filters activating callbacks to the owning game feature plugin
void UModularGameFeaturePluginSubsystem::OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	if (!UModularGameFeaturePluginUtils::IsInGameFeatureModule(this, GameFeatureData))
	{
		return;
	}

	OnGameFeatureInitialize();
}

// Filters deactivating callbacks to the owning game feature plugin
void UModularGameFeaturePluginSubsystem::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	if (!UModularGameFeaturePluginUtils::IsInGameFeatureModule(this, GameFeatureData))
	{
		return;
	}

	OnGameFeatureDeinitialize();
}

// Overridden and marked as final to prevet this subsystem from being created outside a game feature plugin context
bool UModularGameFeaturePluginSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const bool bIsInGameFeature = UModularGameFeaturePluginUtils::IsInAnyGameFeatureModule(this);
	ensureMsgf(bIsInGameFeature, TEXT("ASSERT: [%i] %hs:\n'%s' is not under any registered GameFeatures plugin!"), __LINE__, __FUNCTION__, *GetNameSafe(GetClass()));
	return bIsInGameFeature;
}