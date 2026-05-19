// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_AddWorldPartitionContent.h"

#if WITH_EDITOR
// UE
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/ExternalDataLayerAsset.h"
#include "WorldPartition/DataLayer/ExternalDataLayerEngineSubsystem.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAction_AddWorldPartitionContent)

// Called by the Game Features system when the owning feature is registered
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

#if WITH_EDITOR
	const UExternalDataLayerAsset* DataLayerAsset = GetExternalDataLayerAsset();
	if (!ensureMsgf(DataLayerAsset, TEXT("ASSERT: [%i] %hs:\n'DataLayerAsset' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Activate here instead of in OnGameFeatureActivating: engine generates streaming object only for layers active at play world init, binding it to Registered state keeps streaming object alive across whole session so runtime map switching never loses it
	UExternalDataLayerEngineSubsystem::Get().ActivateExternalDataLayerAsset(DataLayerAsset, this);

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

#endif // WITH_EDITOR
}

// Called by the Game Features system when the owning feature transitions to Active
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureActivating()
{
#if WITH_EDITOR
	// External Data Layer already engine-active since OnGameFeatureRegistering, only stream content in then re-evaluate per-world injection so editor preview shows this now Active map, base would re-cycle engine state and recreate streaming object dependency
	SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState::Activated);
	RefreshInjectionAcrossWorlds();
#else
	Super::OnGameFeatureActivating();
#endif // WITH_EDITOR
}

// Called by the Game Features system when the owning feature is leaving the Active state
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
#if WITH_EDITOR
	// Keep External Data Layer engine-active so play worlds keep streaming object, only stream content out then re-evaluate per-world injection so editor preview drops this now Registered map while play worlds keep it, calling base here would deactivate layer and reintroduce black screen on next runtime map switch
	SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState::Unloaded);
	RefreshInjectionAcrossWorlds();
#else
	Super::OnGameFeatureDeactivating(Context);
#endif // WITH_EDITOR
}

// Called by the Game Features system when the owning feature is unregistered
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureUnregistering()
{
#if WITH_EDITOR
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
#endif // WITH_EDITOR

	Super::OnGameFeatureUnregistering();
}

#if WITH_EDITOR
// Pushes External Data Layer runtime state into every initialized play world so runtime map switching stays in sync
void UGfpmAction_AddWorldPartitionContent::SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState NewState)
{
	const UExternalDataLayerAsset* DataLayerAsset = GetExternalDataLayerAsset();
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
void UGfpmAction_AddWorldPartitionContent::RefreshInjectionAcrossWorlds() const
{
	const UExternalDataLayerAsset* DataLayerAsset = GetExternalDataLayerAsset();
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
void UGfpmAction_AddWorldPartitionContent::HandleOverrideInjection(const UWorld* World, const UExternalDataLayerAsset* InExternalDataLayerAsset, bool& bOutCanInject)
{
	if (InExternalDataLayerAsset != GetExternalDataLayerAsset())
	{
		// Another action owns this layer and decides it, every project layer is a managed map plugin so each is decided by its own action, leave it untouched so multicast order can not conflict
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

	// Editor preview world visibility follows injection, restrict it to the Active map so this one fully unloads on Registered. Treat activating as active so a re-evaluation triggered mid-transition still injects, the persisted editor world is not re-initialized to inject it later
	constexpr bool bCheckForActivating = true;
	bOutCanInject = IsGameFeaturePluginActive(bCheckForActivating);
}

// When play is starting, before engine snapshots the editor world layer set, force-injects own layer into the editor world so its play world streaming object gets generated
void UGfpmAction_AddWorldPartitionContent::OnStartPIE(bool /*bIsSimulating*/)
{
	bForcePiePrepassInjection = true;
	RefreshInjectionAcrossWorlds();
}

// When play world is up and engine already generated streaming objects, restores single-map editor preview while the play world keeps its streaming object
void UGfpmAction_AddWorldPartitionContent::OnPostPIEStarted(bool /*bIsSimulating*/)
{
	if (!bForcePiePrepassInjection)
	{
		// Window not open, nothing to restore
		return;
	}

	// Play world is up and its streaming object already generated. Pre-pass left every layer Activated so the duplicated play worlds inherited it, re-assert content visibility so layers whose feature is not Active unload there, then restore single-map editor preview. Play world keeps its streaming object
	bForcePiePrepassInjection = false;
	constexpr bool bCheckForActivating = true;
	const EDataLayerRuntimeState DesiredState = IsGameFeaturePluginActive(bCheckForActivating) ? EDataLayerRuntimeState::Activated : EDataLayerRuntimeState::Unloaded;
	SetContentRuntimeStateAcrossWorlds(DesiredState);
	RefreshInjectionAcrossWorlds();
}
#endif // WITH_EDITOR
