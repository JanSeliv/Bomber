// Copyright (c) Yevhenii Selivanov

#include "FTGEditorPreviewSubsystem.h"

// FTG
#include "FTGComponent.h"
#include "FTGEditorUtils.h"

/// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"

// UE
#include "Engine/World.h"
#include "GameFeaturesSubsystem.h"
#include "InstancedStaticMeshActor.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FTGEditorPreviewSubsystem)

// Is called when the subsystem is initialized
void UFTGEditorPreviewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);

	// Editor subsystem outlives each opened map, so release the preview component when its world tears down to avoid world GC leaks on map reopen
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanup);

	// Catch the case where the owning plugin is already active before this observer could register
	const FName ModuleName = FName(*UModularGameFeaturePluginUtils::GetModuleNameByObject(this));
	if (UModularGameFeaturePluginUtils::IsModularGameFeatureActive(ModuleName))
	{
		OnGameFeatureInitialize();
	}
}

// Is called when the subsystem is deinitialized
void UFTGEditorPreviewSubsystem::Deinitialize()
{
	// UGameFeaturesSubsystem::Get() asserts on GEngine, which can be torn down before editor subsystems, so resolve defensively
	UGameFeaturesSubsystem* GameFeaturesSubsystem = GEngine ? GEngine->GetEngineSubsystem<UGameFeaturesSubsystem>() : nullptr;
	if (GameFeaturesSubsystem)
	{
		const FName ModuleName = FName(*UModularGameFeaturePluginUtils::GetModuleNameByObject(this));
		if (UModularGameFeaturePluginUtils::IsModularGameFeatureActive(ModuleName))
		{
			OnGameFeatureDeinitialize();
		}

		GameFeaturesSubsystem->RemoveObserver(this);
	}

	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

	Super::Deinitialize();
}

// Filters activating callbacks to the owning game feature plugin
void UFTGEditorPreviewSubsystem::OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	if (!UModularGameFeaturePluginUtils::IsInGameFeatureModule(this, GameFeatureData))
	{
		return;
	}

	OnGameFeatureInitialize();
}

// Filters deactivating callbacks to the owning game feature plugin
void UFTGEditorPreviewSubsystem::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	if (!UModularGameFeaturePluginUtils::IsInGameFeatureModule(this, GameFeatureData))
	{
		return;
	}

	OnGameFeatureDeinitialize();
}

// Subscribes to Generated Map readiness once the owning plugin becomes active
void UFTGEditorPreviewSubsystem::OnGameFeatureInitialize()
{
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GeneratedMap_Ready, this, &ThisClass::OnGeneratedMapReady);
}

// Destroys any spawned preview component and unsubscribes once the owning plugin becomes inactive
void UFTGEditorPreviewSubsystem::OnGameFeatureDeinitialize()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	if (IsValid(FootTrailGenerator))
	{
		// Editor-preview components never get EndPlay, so the spawned actor must be destroyed manually before the component goes away
		if (AInstancedStaticMeshActor* InstancedFootTrailsActor = FootTrailGenerator->GetInstancedStaticMeshActor())
		{
			InstancedFootTrailsActor->Destroy();
		}

		FootTrailGenerator->DestroyComponent();
		FootTrailGenerator = nullptr;
	}
}

// Releases preview component reference when its owning world is torn down so GC can collect the outgoing world
void UFTGEditorPreviewSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (!IsValid(FootTrailGenerator)
	    || FootTrailGenerator->GetWorld() != World)
	{
		return;
	}

	// Outgoing world destroys the component itself, just release the reference here so it no longer pins the world
	FootTrailGenerator = nullptr;
}

/// Called when Generated Map is initialized and its data assets are loaded, is also called in editor
void UFTGEditorPreviewSubsystem::OnGeneratedMapReady_Implementation(const FGameplayEventData& Payload)
{
	if (!UUtilsLibrary::IsEditorNotPieWorld() // Only preview in editor, not in PIE
	    || IsValid(FootTrailGenerator)) // skip if already initialized
	{
		return;
	}

	const TSubclassOf<UFTGComponent> ComponentClass = UFTGEditorUtils::GetFootTrailsComponentClass();
	if (!ensureMsgf(ComponentClass, TEXT("ASSERT: [%i] %hs:\n'ComponentClass' is not found! Make sure '%s' class is assigned in MGF data asset"), __LINE__, __FUNCTION__, *UFTGComponent::StaticClass()->GetName()))
	{
		return;
	}

	FootTrailGenerator = NewObject<UFTGComponent>(ABmrGeneratedMap::GetGeneratedMap(), ComponentClass, NAME_None, RF_Transient);
	FootTrailGenerator->RegisterComponent();
	FootTrailGenerator->InitOnce();

	// Hide editor-only version of trails in PIE
	AInstancedStaticMeshActor* InstancedFootTrailsActor = FootTrailGenerator->GetInstancedStaticMeshActor();
	checkf(InstancedFootTrailsActor, TEXT("ERROR: [%i] %hs:\n'InstancedFootTrailsActor' is null!"), __LINE__, __FUNCTION__);
	InstancedFootTrailsActor->SetActorHiddenInGame(true);
}
