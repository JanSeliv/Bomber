// Copyright (c) Yevhenii Selivanov

#pragma once

#include "ActionObservers/GfpmActionObserver_Base.h"

#include "GfpmActionObserver_AddWorldPartitionContent.generated.h"

enum class EDataLayerRuntimeState : uint8;

/**
 * Editor\-game observer that drives per-world External Data Layer injection for the Add World Partition Content action.
 * Keeps editor preview showing only the Active map while play worlds keep the engine default and the play-start window force-injects, so runtime map switching never loses the streaming object.
 */
UCLASS()
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmActionObserver_AddWorldPartitionContent final : public UGfpmActionObserver_Base
{
	GENERATED_BODY()

public:
	/** Identifies the action type this observer handles. */
	virtual TSubclassOf<class UGameFeatureAction> GetObservedActionClass() const override;

	/** External Data Layer injection must run under -game as well, so this observer is created in every configuration. */
	FORCEINLINE virtual bool ShouldCreateObserver() const override { return true; }

	/** When owning plugin is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** When owning plugin begins activating. */
	virtual void OnGameFeatureActivating() override;

	/** When owning plugin begins deactivating. */
	virtual void OnGameFeatureDeactivating() override;

	/** When owning plugin is unregistering. */
	virtual void OnGameFeatureUnregistering() override;

protected:
	/** Pushes External Data Layer runtime state into every initialized world so runtime map switching stays in sync. */
	void SetContentRuntimeStateAcrossWorlds(EDataLayerRuntimeState NewState);

	/** Re-broadcasts own External Data Layer engine state without changing it, so every world re-evaluates its injection decision. */
	void RefreshInjectionAcrossWorlds() const;

	/** Per-world injection decision for own layer, keeps editor preview showing only the Active map while play worlds keep engine default and the play start window force-injects. */
	void HandleOverrideInjection(const class UWorld* World, const class UExternalDataLayerAsset* InExternalDataLayerAsset, bool& bOutCanInject);

	/** When play is starting, force-injects own layer into the editor world so its play world streaming object gets generated. */
	void OnStartPIE(bool bIsSimulating);

	/** When play world is up, restores single-map editor preview while the play world keeps its streaming object. */
	void OnPostPIEStarted(bool bIsSimulating);

	/** Returns observed action's External Data Layer asset. */
	const class UExternalDataLayerAsset* GetObservedDataLayerAsset() const;

	/** Handle for own injection override binding, every observer binds its own and only decides its own layer so multicast order can not conflict. */
	FDelegateHandle OverrideInjectionHandle;

	/** Handle for own play start binding, force-injection window opens here. */
	FDelegateHandle StartPIEHandle;

	/** Handle for own play started binding, force-injection window closes here. */
	FDelegateHandle PostPIEStartedHandle;

	/** True only during the play start window, force-allows editor world injection of own layer so its streaming object gets generated into the play start snapshot. */
	bool bForcePiePrepassInjection = false;
};
