// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// UE
#include "Engine/TimerHandle.h"
#include "GameplayTagContainer.h"

#include "GfpmLoaderSubsystem.generated.h"

/**
 * Per-world subsystem that drives tag-driven Game Feature Plugin (GFP) loading/unloading.
 * One instance lives in every relevant world (editor, PIE server and clients, dedicated game), each watches own Ability System Component and routes activation through own world context.
 * Registry populated from UGfpmAction_RegisterGameFeaturePluginActivation entries in each plugin's GameFeatureData.
 */
UCLASS(DisplayName = "Game Feature Plugins Loader Subsystem")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmLoaderSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this subsystem for current play world, checked. Crashes if unavailable */
	static UGfpmLoaderSubsystem& Get();

	/** Returns this subsystem for current play world or nullptr */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static UGfpmLoaderSubsystem* GetLoaderSubsystem();

	/** Whether any tag-driven GFP is pending activation despite owning required tags on tracked ASC */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	bool HasPendingTagDrivenActivations() const;

	/** Collects union of plugin names appearing across every entry of this instance's registry. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	void GetAllRegisteredPlugins(TArray<FName>& OutNames) const;

	/** Registers or replaces this world's activation entry of owning Game Feature Plugin resolved from given GameFeatureData, then schedules apply. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void RegisterGameFeaturePluginActivation(const class UGameFeatureData* GameFeatureData, const FGameplayTagContainer& ActivationTags);

	/** Drops this world's activation entry of owning Game Feature Plugin resolved from given GameFeatureData. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void UnregisterGameFeaturePluginActivation(const class UGameFeatureData* GameFeatureData);

	/*********************************************************************************************
	 * Tag-Driven Features
	 ********************************************************************************************* */
protected:
	/** When this world's ASC becomes available and ready to broadcast tags */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnWorldASCReady(const struct FGameplayEventData& Payload);

	/** Ability System Component tracked for this world, only one per world */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Transient, Category = "[Game Feature Plugins Manager]")
	TObjectPtr<class UAbilitySystemComponent> TrackedAsc = nullptr;

	/** Generic tag-event delegate handle for tracked ASC */
	FDelegateHandle TagEventHandle;

	/** Pending next-tick recompute handle, coalesces tag-event bursts */
	FTimerHandle DeferredRecomputeHandle;

	/** Per-world registry of GFPs to activate by tag */
	TMap<FGameplayTag, TArray<FName>> PluginsByTag;

	/** Drops registry entries pointing at given plugin from this instance only */
	void RemovePluginFromRegistry(FName PluginName);

	/** When tracked ASC's tag count changes.
	 * @param ChangedTag Tag whose count just transitioned.
	 * @param NewCount Updated occurrence count on the ASC for ChangedTag. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnAscTagCountChanged(FGameplayTag ChangedTag, int32 NewCount);

	/** Defers GFPs applying to next tick, coalescing burst tag events into one pass once tags settle.
	 * @param bAllowEmptyAggregate false skips apply on empty aggregate (startup path before any tag observed), true forces apply on empty aggregate so explicit tag removal can deactivate plugins. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void ScheduleApplyGameFeatures(bool bAllowEmptyAggregate);

	/** Recomputes desired feature set from this world's ASC tags and applies activation/deactivation delta. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void ApplyGameFeatures(bool bAllowEmptyAggregate);

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** When subsystem initializes. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** When subsystem is destroyed. */
	virtual void Deinitialize() override;
};
