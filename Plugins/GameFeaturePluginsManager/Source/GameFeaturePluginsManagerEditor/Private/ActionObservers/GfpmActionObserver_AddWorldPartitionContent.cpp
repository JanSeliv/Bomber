// Copyright (c) Yevhenii Selivanov

#include "ActionObservers/GfpmActionObserver_AddWorldPartitionContent.h"

// GFPM
#include "GameFeatureActions/GfpmAction_AddWorldPartitionContent.h"

// UE
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeatureAction.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/ExternalDataLayerAsset.h"
#include "WorldPartition/DataLayer/ExternalDataLayerEngineSubsystem.h"
#include "WorldPartition/WorldPartition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmActionObserver_AddWorldPartitionContent)

// Identifies the action type this observer handles
TSubclassOf<UGameFeatureAction> UGfpmActionObserver_AddWorldPartitionContent::GetObservedActionClass() const
{
	return UGfpmAction_AddWorldPartitionContent::StaticClass();
}

// When owning plugin is registered
void UGfpmActionObserver_AddWorldPartitionContent::OnGameFeatureRegistering()
{
	if (!OverrideInjectionHandle.IsValid())
	{
		OverrideInjectionHandle = UExternalDataLayerEngineSubsystem::Get().OnExternalDataLayerOverrideInjection.AddUObject(this, &ThisClass::HandleOverrideInjection);
	}
	if (!StartPIEHandle.IsValid())
	{
		StartPIEHandle = FEditorDelegates::StartPIE.AddUObject(this, &ThisClass::OnStartPIE);
	}
	if (!PostPIEStartedHandle.IsValid())
	{
		PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddUObject(this, &ThisClass::OnPostPIEStarted);
	}
	if (!PostWorldInitHandle.IsValid())
	{
		// Reopened game worlds fire no GFP transition, so catch world init here to re-assert content there
		PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this, [this](UWorld* World, const UWorld::InitializationValues /*IVS*/)
		{
			OnGameWorldInitialized(World);
		});
	}
}

// When owning plugin begins activating
void UGfpmActionObserver_AddWorldPartitionContent::OnGameFeatureActivating()
{
	// External Data Layer already engine-active since OnGameFeatureRegistering, only stream content in then re-evaluate per-world injection so editor preview shows this now Active map
	SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState::Activated);
	RefreshInjectionAcrossWorlds();
}

// When owning plugin begins deactivating
void UGfpmActionObserver_AddWorldPartitionContent::OnGameFeatureDeactivating()
{
	// Keep External Data Layer engine-active so play worlds keep streaming object, only stream content out then re-evaluate per-world injection so editor preview drops this now Registered map while play worlds keep it
	SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState::Unloaded);
	RefreshInjectionAcrossWorlds();
}

// When owning plugin is unregistering
void UGfpmActionObserver_AddWorldPartitionContent::OnGameFeatureUnregistering()
{
	if (OverrideInjectionHandle.IsValid())
	{
		UExternalDataLayerEngineSubsystem::Get().OnExternalDataLayerOverrideInjection.Remove(OverrideInjectionHandle);
		OverrideInjectionHandle.Reset();
	}
	if (StartPIEHandle.IsValid())
	{
		FEditorDelegates::StartPIE.Remove(StartPIEHandle);
		StartPIEHandle.Reset();
	}
	if (PostPIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);
		PostPIEStartedHandle.Reset();
	}
	if (PostWorldInitHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
		PostWorldInitHandle.Reset();
	}
}

// Pushes External Data Layer runtime state into every initialized world so runtime map switching stays in sync
void UGfpmActionObserver_AddWorldPartitionContent::SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState NewState)
{
	const UExternalDataLayerAsset* DataLayerAsset = GetObservedDataLayerAsset();
	if (!ensureMsgf(DataLayerAsset && GEngine, TEXT("ASSERT: [%i] %hs:\n'DataLayerAsset && GEngine' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(WorldContext.World());
		if (!DataLayerManager)
		{
			// World without World Partition data layers, nothing to drive
			continue;
		}

		constexpr bool bIsRecursive = true;
		DataLayerManager->SetDataLayerRuntimeState(DataLayerAsset, NewState, bIsRecursive);
	}
}

// Re-broadcasts own External Data Layer engine state without changing it, so every world re-evaluates its injection decision
void UGfpmActionObserver_AddWorldPartitionContent::RefreshInjectionAcrossWorlds() const
{
	const UExternalDataLayerAsset* DataLayerAsset = GetObservedDataLayerAsset();
	if (!ensureMsgf(DataLayerAsset, TEXT("ASSERT: [%i] %hs:\n'DataLayerAsset' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Re-broadcast current engine-active state without changing it, so every world re-runs its injection decision through HandleOverrideInjection
	UExternalDataLayerEngineSubsystem& Subsystem = UExternalDataLayerEngineSubsystem::Get();
	const EExternalDataLayerRegistrationState State = Subsystem.GetExternalDataLayerAssetRegistrationState(DataLayerAsset);
	Subsystem.OnExternalDataLayerAssetRegistrationStateChanged.Broadcast(DataLayerAsset, State, State);
}

// Per-world injection decision for own layer, keeps editor preview showing only the Active map while play worlds keep engine default and the play start window force-injects
void UGfpmActionObserver_AddWorldPartitionContent::HandleOverrideInjection(const UWorld* World, const UExternalDataLayerAsset* InExternalDataLayerAsset, bool& bOutCanInject)
{
	if (InExternalDataLayerAsset != GetObservedDataLayerAsset())
	{
		// Another observer owns this layer and decides it, every project layer is a managed map plugin so each is decided by its own observer, leave it untouched so multicast order can not conflict
		return;
	}

	// Binding this delegate bypasses the engine default for own layer, so reproduce it first then narrow below
	bOutCanInject = UExternalDataLayerEngineSubsystem::Get().IsExternalDataLayerAssetActive(InExternalDataLayerAsset);

	if (!World
	    || World->IsGameWorld())
	{
		// Play world must keep engine default so its streaming object stays resident for runtime map switching
		return;
	}

	if (bForcePiePrepassInjection)
	{
		// Play start streaming pre-generation pass, force-allow editor world injection so this layer's container registers and its play world streaming object gets generated
		bOutCanInject = true;
		return;
	}

	// Editor preview world visibility follows injection, restrict it to the Active map so this one fully unloads on Registered. Treat activating as active so a re-evaluation triggered mid-transition still injects
	constexpr bool bCheckForActivating = true;
	const UGameFeatureAction* Action = ObservedAction.Get();
	bOutCanInject = Action && Action->IsGameFeaturePluginActive(bCheckForActivating);
}

// When play is starting, force-injects own layer into the editor world so its play world streaming object gets generated
void UGfpmActionObserver_AddWorldPartitionContent::OnStartPIE(bool /*bIsSimulating*/)
{
	bForcePiePrepassInjection = true;
	RefreshInjectionAcrossWorlds();
}

// When play world is up, restores single-map editor preview while the play world keeps its streaming object
void UGfpmActionObserver_AddWorldPartitionContent::OnPostPIEStarted(bool /*bIsSimulating*/)
{
	if (!bForcePiePrepassInjection)
	{
		// Window not open, nothing to restore
		return;
	}

	// Play world is up and its streaming object already generated, re-assert content visibility so layers whose feature is not Active unload there, then restore single-map editor preview
	bForcePiePrepassInjection = false;
	constexpr bool bCheckForActivating = true;
	const UGameFeatureAction* Action = ObservedAction.Get();
	const bool bIsActive = Action && Action->IsGameFeaturePluginActive(bCheckForActivating);
	const EDataLayerRuntimeState DesiredState = bIsActive ? EDataLayerRuntimeState::Activated : EDataLayerRuntimeState::Unloaded;
	SetContentRuntimeStateAcrossWorlds(DesiredState);
	RefreshInjectionAcrossWorlds();
}

// When any world finished initializing, binds reopened game worlds so their partition-init can re-assert content
void UGfpmActionObserver_AddWorldPartitionContent::OnGameWorldInitialized(UWorld* World)
{
	if (!World
	    || !World->IsGameWorld())
	{
		// Editor preview worlds are driven by GFP lifecycle hooks, only reopened game worlds need re-assert
		return;
	}

	// World partition not initialized yet here, bind its initialized event which fires right after External Data Layer injection
	World->OnWorldPartitionInitialized().RemoveAll(this);
	World->OnWorldPartitionInitialized().AddUObject(this, &ThisClass::OnWorldPartitionInitialized);
}

// When reopened game world's partition is initialized, re-asserts own content so world drops this layer when its feature is not Active
void UGfpmActionObserver_AddWorldPartitionContent::OnWorldPartitionInitialized(UWorldPartition* WorldPartition)
{
	const UWorld* World = WorldPartition ? WorldPartition->GetWorld() : nullptr;
	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(World);
	const UExternalDataLayerAsset* DataLayerAsset = GetObservedDataLayerAsset();
	if (!DataLayerManager
	    || !DataLayerAsset)
	{
		// World without data layers, or observed action already gone, nothing to drive
		return;
	}

	// Engine seeds every injected External Data Layer at its Activated initial state, so reopened world comes up with all maps loaded. Re-assert content only to this map's feature state, content never engine-deactivates so session-active layer keeps its streaming object
	constexpr bool bCheckForActivating = true;
	const UGameFeatureAction* Action = ObservedAction.Get();
	const bool bIsActive = Action && Action->IsGameFeaturePluginActive(bCheckForActivating);
	const EDataLayerRuntimeState DesiredState = bIsActive ? EDataLayerRuntimeState::Activated : EDataLayerRuntimeState::Unloaded;
	constexpr bool bIsRecursive = true;
	DataLayerManager->SetDataLayerRuntimeState(DataLayerAsset, DesiredState, bIsRecursive);
}

// Returns observed action's External Data Layer asset
const UExternalDataLayerAsset* UGfpmActionObserver_AddWorldPartitionContent::GetObservedDataLayerAsset() const
{
	const UGfpmAction_AddWorldPartitionContent* Action = Cast<UGfpmAction_AddWorldPartitionContent>(ObservedAction.Get());
	return Action ? Action->GetExternalDataLayerAsset() : nullptr;
}
