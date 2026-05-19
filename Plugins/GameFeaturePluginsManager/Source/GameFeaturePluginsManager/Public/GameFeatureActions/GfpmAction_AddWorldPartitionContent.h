// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction_AddWorldPartitionContent.h"

// UE
#include "Delegates/IDelegateInstance.h" // FDelegateHandle

#include "GfpmAction_AddWorldPartitionContent.generated.h"

enum class EDataLayerRuntimeState : uint8;

/**
 * Game Feature action that replaces engine Add World Partition Content action, adding runtime map switching support for External Data Layer content across editor preview, PIE, -game, and cook.
 */
UCLASS(DisplayName = "GFPM Add World Partition Content")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAction_AddWorldPartitionContent final : public UGameFeatureAction_AddWorldPartitionContent
{
	GENERATED_BODY()

protected:
	/** Called by the Game Features system when the owning feature is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** Called by the Game Features system when the owning feature transitions to Active. */
	virtual void OnGameFeatureActivating() override;

	/** Called by the Game Features system when the owning feature is leaving the Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	/** Called by the Game Features system when the owning feature is unregistered. */
	virtual void OnGameFeatureUnregistering() override;

#if WITH_EDITOR
	/** Pushes External Data Layer runtime state into every initialized play world so runtime map switching stays in sync. */
	void SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState NewState);

	/** Re-broadcasts own External Data Layer engine state without changing it, so every world re-evaluates its injection decision. */
	void RefreshInjectionAcrossWorlds() const;

	/** Per-world injection decision for own layer, keeps editor preview showing only the Active map while play worlds keep engine default and the play start window force-injects. */
	void HandleOverrideInjection(const class UWorld* World, const class UExternalDataLayerAsset* InExternalDataLayerAsset, bool& bOutCanInject);

	/** When play is starting, before engine snapshots the editor world layer set, force-injects own layer into the editor world so its play world streaming object gets generated. */
	void OnStartPIE(bool bIsSimulating);

	/** When play world is up and engine already generated streaming objects, restores single-map editor preview while the play world keeps its streaming object. */
	void OnPostPIEStarted(bool bIsSimulating);

	/** Handle for own injection override binding, every action binds its own and only decides its own layer so multicast order can not conflict. */
	FDelegateHandle OverrideInjectionHandle;

	/** Handle for own play start binding, force-injection window opens here. */
	FDelegateHandle StartPIEHandle;

	/** Handle for own play started binding, force-injection window closes here. */
	FDelegateHandle PostPIEStartedHandle;

	/** True only during the play start window, force-allows editor world injection of own layer so its streaming object gets generated into the play start snapshot. */
	bool bForcePiePrepassInjection = false;
#endif // WITH_EDITOR
};
