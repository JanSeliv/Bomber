// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"

// UE
#include "Engine/TimerHandle.h"
#include "GameplayTagContainer.h"

#include "GfpmLoaderSubsystem.generated.h"

/**
 * Manages tag-driven Game Feature Plugin (GFP) loading/unloading across all worlds (editor, PIE, cook).
 * Features activate when their required tags appear on world ASC, deactivate when tags are removed.
 * Features are populated from UGfpmAction_RegisterGameFeaturePluginActivation entries living inside each plugin's GameFeatureData.
 */
UCLASS(DisplayName = "Game Feature Plugins Loader Subsystem")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmLoaderSubsystem final : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this subsystem, checked. Crashes if unavailable */
	static UGfpmLoaderSubsystem& Get();

	/** Returns this subsystem or nullptr */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static UGfpmLoaderSubsystem* GetLoaderSubsystem();

	/** Whether any tag-driven GFP is pending activation despite owning required tags */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	bool HasPendingTagDrivenActivations() const;

	/** Collects the union of plugin names appearing across every entry of PluginsByTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	void GetAllRegisteredPlugins(TArray<FName>& OutNames) const;

	/** Registers or replaces activation entry of owning Game Feature Plugin resolved from given GameFeatureData. Schedules apply when any ASC is already tracked. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void RegisterGameFeaturePluginActivation(const class UGameFeatureData* GameFeatureData, const FGameplayTagContainer& ActivationTags);

	/** Drops activation entry of owning Game Feature Plugin resolved from given GameFeatureData. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void UnregisterGameFeaturePluginActivation(const class UGameFeatureData* GameFeatureData);

	/*********************************************************************************************
	 * Tag-Driven Features
	 ********************************************************************************************* */
protected:
	/** When a world's ASC becomes available and ready to broadcast tags */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnWorldASCReady(const struct FGameplayEventData& Payload);

	/** Per-ASC generic-tag-event delegate handles; presence in this map means the ASC is tracked */
	TMap<TWeakObjectPtr<class UAbilitySystemComponent>, FDelegateHandle> TrackedAscs;

	/** Contains registered GFPs for auto activation and deactivation by specified tags. */
	TMap<FGameplayTag, TArray<FName>> PluginsByTag;

	/** Pending next-tick recompute handle; coalesces tag-event bursts */
	FTimerHandle DeferredRecomputeHandle;

	/** When an ASC's tag count changes */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnAscTagCountChanged(FGameplayTag ChangedTag, int32 NewCount, UAbilitySystemComponent* SourceAsc);

	/** Defers GFPs applying to next tick.
	 * By design during one frame multiple tags might arrive independently, applying one and immediately removing it,
	 * so we want to avoid unnecessary intermediate state changes and only react once the tags are settled. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, WorldContext = "WorldContextObject"))
	void ScheduleApplyGameFeatures(const UObject* WorldContextObject);

	/** Recomputes desired feature set and applies activation/deactivation delta */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void ApplyGameFeatures();

	/** Whether ASC's world is authoritative for GFP decisions */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	bool IsAuthoritativeAsc(const UAbilitySystemComponent* ASC) const;

	/*********************************************************************************************
	 * World Lifecycle
	 ********************************************************************************************* */
protected:
	/** When a world is about to finish destruction */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnPreWorldFinishDestroy(class UWorld* World);

#if WITH_EDITOR
	/** When editor finishes shutting down PIE; forces feature cycle (unload then reload from editor authority) */
	void OnEditorShutdownPIE(bool bIsSimulating);
#endif // WITH_EDITOR

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** When subsystem initializes */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** When subsystem is destroyed */
	virtual void Deinitialize() override;
};
