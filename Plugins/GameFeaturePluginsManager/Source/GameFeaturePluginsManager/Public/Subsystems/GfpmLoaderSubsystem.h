// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"

// UE
#include "Containers/Ticker.h"
#include "GameplayTagContainer.h"

#include "GfpmLoaderSubsystem.generated.h"

/**
 * Manages tag-driven Modular Game Feature loading/unloading across all worlds (editor, PIE, cook).
 * Features activate when their required tags appear on world ASC, deactivate when tags are removed.
 * Features are defined in UGfpmSettings.
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
	static UGfpmLoaderSubsystem* GetModularGameFeaturesSubsystem();

	/** Whether any tag-driven MGF is pending activation despite owning required tags */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	bool HasPendingTagDrivenActivations() const;

	/*********************************************************************************************
	 * Tag-Driven Features
	 ********************************************************************************************* */
protected:
	/** When a world's ASC becomes available and ready to broadcast tags */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnWorldASCReady(const struct FGameplayEventData& Payload);

	/** Per-ASC tag snapshots across all tracked worlds; only authoritative ASCs drive feature decisions */
	TMap<TWeakObjectPtr<class UAbilitySystemComponent>, FGameplayTagContainer> AscOwnedTags;

	/** Features currently active via this subsystem; used to compute activation/deactivation delta */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, AdvancedDisplay, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	TSet<FName> ActiveFeatures;

	/** Pending next-tick recompute handle; coalesces tag-event bursts */
	FTSTicker::FDelegateHandle DeferredRecomputeHandle;

	/** When an ASC's tag count changes */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnAscTagCountChanged(FGameplayTag ChangedTag, int32 NewCount, UAbilitySystemComponent* SourceAsc);

	/** Defers feature set recomputation to next tick */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void QueueDeferredRecompute();

	/** Recomputes desired feature set and applies activation/deactivation delta */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void ApplyAuthoritativeFeatureSet();

	/** Whether ASC's world is authoritative for MGF decisions */
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
	/** Whether active features are frozen during editor<->PIE transition, released when first authoritative tag event fires */
	bool bIsAuthorityTransitioning = false;

	/** When Play In Editor is about to begin */
	void OnPreBeginPIE(bool bIsSimulating);

	/** When Play In Editor ends */
	void OnEndPIE(bool bIsSimulating);

	/** When editor finishes shutting down PIE; forces feature cycle (unload then reload from editor authority) */
	void OnEditorShutdownPIE(bool bIsSimulating);
#endif

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** When subsystem initializes */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** When subsystem is destroyed */
	virtual void Deinitialize() override;
};
