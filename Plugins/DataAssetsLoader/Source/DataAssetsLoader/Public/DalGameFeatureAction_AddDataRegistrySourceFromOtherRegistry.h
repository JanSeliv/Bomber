// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "GameFeatureAction_DataRegistrySource.h"
#include "UObject/ObjectPtr.h"

#include "DalGameFeatureAction_AddDataRegistrySourceFromOtherRegistry.generated.h"

/**
 * Pairs a Data Registry source spec with the row struct of an upstream registry that gates its application.
 * The action waits until the upstream registry has at least one cached row, then applies Source.
 * Symmetric: when the upstream registry empties, Source is removed again.
 */
USTRUCT()
struct DATAASSETSLOADER_API FDalDataRegistrySourceWithDependency
{
	GENERATED_BODY()

	/** Row struct of the upstream Data Registry whose cached rows gate Source. */
	UPROPERTY(EditAnywhere, Category = "Dependency")
	TObjectPtr<UScriptStruct> DependsOnRowStruct = nullptr;

	/** Source spec applied to the Data Registry while DependsOnRowStruct's registry has rows. */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (ShowOnlyInnerProperties))
	FDataRegistrySourceToAdd Source;
};

/**
 * Game Feature action that applies Data Registry sources gated by the load state of other Data Registries.
 * Each entry pairs a target source with the row struct of an upstream registry, the source is applied as soon as the upstream registry reports cached rows and removed when the upstream registry empties.
 * Listens passively via UDataRegistry::OnCacheVersionInvalidated, never triggers loading itself, upstream loads must be driven by some other action (engine AddDataRegistrySource, asset bundles, etc).
 * Late-loaded registries that appear after subsystem init are not picked up automatically; if the upstream registry only exists after the action activates, deactivate and reactivate the feature.
 */
UCLASS(meta = (DisplayName = "DAL Add Data Registry Source (From Other Registry)"))
class DATAASSETSLOADER_API UDalGameFeatureAction_AddDataRegistrySourceFromOtherRegistry final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** Sources applied or removed in lockstep with their upstream registries' cached row counts. */
	UPROPERTY(EditAnywhere, Category = "Registry Data", meta = (TitleProperty = "DependsOnRowStruct"))
	TArray<FDalDataRegistrySourceWithDependency> SourcesToAdd;

protected:
	/** Called by the Game Features system when the owning feature transitions to Active. */
	virtual void OnGameFeatureActivated() override;

	/** Called by the Game Features system when the owning feature is leaving the Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITOR
	/** Reports configuration errors to the editor's Data Validation system: empty list, missing dep struct, and per-source field validity. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	/*********************************************************************************************
	 * Internal
	 ********************************************************************************************* */
protected:
	/** Hook bound to UDataRegistrySubsystem::OnSubsystemInitialized when activation occurs before registries are ready; once fired, resolution proceeds. */
	void OnDataRegistrySubsystemInitialized();

	/** Walks unique upstream row structs, resolves each to its registry via DAL and subscribes to its cache invalidation; runs an immediate evaluation snapshot afterwards. */
	void ResolveAndSubscribeAll();

	/** Hook bound to each upstream UDataRegistry::OnCacheVersionInvalidated, defers all per-entry decisions to EvaluateAllEntries. */
	void OnRegistryCacheInvalidated(class UDataRegistry* InRegistry);

	/** Re-checks every entry: applies the source on the rising edge (rows become non-empty) and removes it on the falling edge (rows become empty). */
	void EvaluateAllEntries();

	/** Applies the entry's Source to the Data Registry honoring per-entry client/server flags via the active project policy. */
	void ApplyEntry(int32 EntryIndex);

	/** Removes the entry's Source from the Data Registry, mirroring ApplyEntry's effect. */
	void RemoveEntry(int32 EntryIndex);

	/** Removes every entry whose flag is set; used during deactivation. */
	void RemoveAllApplied();

	/** Drops every per-registry cache subscription held by this action. */
	void ClearAllRegistrySubscriptions();

	/** Handle for the FDataRegistrySubsystemInitializedCallback binding when activation happens before subsystem init. */
	FDelegateHandle OnSubsystemInitHandle;

	/** Per-upstream-registry cache invalidation handles, kept alive while the feature is Active. */
	TMap<TWeakObjectPtr<class UDataRegistry>, FDelegateHandle> RegistryHandles;

	/** Parallel to SourcesToAdd, bit set means the entry's Source is currently applied. */
	TBitArray<> AppliedFlags;
};