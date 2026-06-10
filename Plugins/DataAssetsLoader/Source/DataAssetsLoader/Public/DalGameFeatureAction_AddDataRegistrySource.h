// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "UObject/ObjectPtr.h"
#include "UObject/SoftObjectPtr.h"

#include "DalGameFeatureAction_AddDataRegistrySource.generated.h"

/**
 * Single Data Registry source spec applied by UDalGameFeatureAction_AddDataRegistrySource.
 * Target registry is resolved from its row struct via DAL, so no registry name is ever stored.
 * Optionally gated by another Data Registry's load state, and optionally driven at registration instead of activation.
 */
USTRUCT(BlueprintType)
struct DATAASSETSLOADER_API FDalDataRegistrySource
{
	GENERATED_BODY()

	/** Target Data Registry this source is added to, identified by its row struct. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]", meta = (MetaStruct = "/Script/Engine.TableRowBase", ExcludeBaseStruct))
	TObjectPtr<UScriptStruct> DataRegistry = nullptr;

	/** Data table asset added to target Data Registry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]")
	TSoftObjectPtr<class UDataTable> DataTableToAdd = nullptr;

	/** Curve table asset added to target Data Registry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]")
	TSoftObjectPtr<class UCurveTable> CurveTableToAdd = nullptr;

	/** Search priority within registry, higher priorities are searched first. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]", meta = (ClampMin = "0", ClampMax = "255"))
	int32 AssetPriority = 0;

	/** Whether this source is added on clients. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]")
	bool bClientSource = true;

	/** Whether this source is added on servers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]")
	bool bServerSource = true;

	/** Optional upstream Data Registry whose cached rows gate this source: applied only while that registry has rows, removed when it empties.
	 * Leave empty to apply unconditionally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]", meta = (DisplayName = "(Optional) Depends On Data Registry", MetaStruct = "/Script/Engine.TableRowBase", ExcludeBaseStruct))
	TObjectPtr<UScriptStruct> OptionalDependsOnDataRegistry = nullptr;

	/** When true, source is applied/removed during registration/unregistration instead of activation/deactivation, so dependents can resolve rows before feature fully activates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]")
	bool bPreloadOnRegistered = false;
};

/**
 * Game Feature action that adds Data Registry sources, identifying each target registry by its row struct.
 * Each source can optionally be gated by upstream registry load state or driven at registration instead of activation.
 * Sources are process-global and applied once per plugin transition via UDataRegistrySubsystem.
 */
UCLASS(meta = (DisplayName = "DAL Add Data Registry Source"))
class DATAASSETSLOADER_API UDalGameFeatureAction_AddDataRegistrySource final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** Sources added by this action, each resolves its target registry from its row struct. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[DataAssetsLoader]", meta = (TitleProperty = "DataRegistry"))
	TArray<FDalDataRegistrySource> SourcesToAdd;

protected:
	/** Called by Game Features system when owning feature is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** Called by Game Features system when owning feature is unregistered. */
	virtual void OnGameFeatureUnregistering() override;

	/** Called by Game Features system when owning feature transitions to Active. */
	virtual void OnGameFeatureActivating() override;

	/** Called by Game Features system when owning feature is leaving Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITORONLY_DATA
	/** Called when owning GameFeatureData rebuilds its asset bundles on save. */
	virtual void AddAdditionalAssetBundleData(struct FAssetBundleData& AssetBundleData) override;
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
	/** Called by editor Data Validation system when this asset is validated. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

	/*********************************************************************************************
	 * Internal
	 ********************************************************************************************* */
protected:
	/** Begins subset of sources matching given phase: applies ungated ones and subscribes gated ones. */
	void BeginSourcesForPhase(bool bPreloadPhase);

	/** Ends subset of sources matching given phase: removes applied ones and drops gate subscriptions when no gated source remains. */
	void EndSourcesForPhase(bool bPreloadPhase);

	/** Hook bound to UDataRegistrySubsystem::OnSubsystemInitialized when phase begins before registries are ready. */
	void OnDataRegistrySubsystemInitialized();

	/** Applies begun ungated sources and subscribes begun gated sources to their upstream registry cache invalidation, then runs immediate evaluation. */
	void ResolveAndSubscribeAll();

	/** Hook bound to each upstream UDataRegistry::OnCacheVersionInvalidated, defers per-entry evaluation on cache change. */
	void OnRegistryCacheInvalidated(class UDataRegistry* InRegistry);

	/** Re-checks every begun gated entry: applies on rising edge (rows become non-empty), removes on falling edge. */
	void EvaluateAllEntries();

	/** Adds source to its target registry, registry resolved from its row struct, honoring client/server flags.
	 * @return true when source was actually preregistered, false when excluded by client/server gate. */
	bool ApplyEntry(int32 EntryIndex);

	/** Removes source from its target registry, mirroring ApplyEntry. */
	void RemoveEntry(int32 EntryIndex);

	/** Drops every per-registry cache subscription held by this action. */
	void ClearAllRegistrySubscriptions();

	/** Returns true when at least one gated source is currently begun, so its registry subscriptions must stay alive. */
	bool HasBegunGatedSources() const;

	/** Resolves target registry type for source row struct via DAL, or none type when unresolved. */
	static struct FDataRegistryType GetRegistryTypeForStruct(const UScriptStruct* InStruct);

	/** Handle for subsystem-initialized binding when phase begins before subsystem init. */
	FDelegateHandle OnSubsystemInitHandle;

	/** Per-upstream-registry cache invalidation handles, kept alive while any dependent source is begun. */
	TMap<TWeakObjectPtr<class UDataRegistry>, FDelegateHandle> RegistryHandles;

	/** Parallel to SourcesToAdd, bit set means entry has entered its lifecycle phase and is eligible for evaluation. */
	TBitArray<> BegunFlags;

	/** Parallel to SourcesToAdd, bit set means gated entry source is currently applied to its registry. */
	TBitArray<> AppliedFlags;
};
