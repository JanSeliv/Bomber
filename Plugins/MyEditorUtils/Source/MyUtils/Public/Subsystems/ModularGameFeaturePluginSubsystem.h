// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// UE
#include "GameFeatureStateChangeObserver.h"

#include "ModularGameFeaturePluginSubsystem.generated.h"

/**
 * Base world subsystem for Modular Game Feature (MGF) plugins.
 * In comparison to other world subsystems, it properly triggers on each game feature load and unload.
 * Subclasses override OnGameFeatureInitialize/OnGameFeatureDeinitialize instead of OnWorldBeginPlay/OnWorldEndPlay/Initialize/Deinitialize.
 * This subsystem should be created only under game feature plugin modules.
 * Call SetTickable to enable/disable tick (is disabled by default)
 */
UCLASS(Abstract)
class MYUTILS_API UModularGameFeaturePluginSubsystem : public UTickableWorldSubsystem
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

protected:
	/** Called when the owning game feature plugin activates or on first world begin play.
	 * Override to subscribe to events and initialize retriggerable state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameFeatureInitialize();
	virtual void OnGameFeatureInitialize_Implementation() { }

	/** Called when the owning game feature plugin deactivates or on world end play.
	 * Override to clean up transient state and unsubscribe from events */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameFeatureDeinitialize();
	virtual void OnGameFeatureDeinitialize_Implementation() { }

	/*********************************************************************************************
	 * INTERNAL: do not override these in child classes, use OnGameFeatureInitialize/OnGameFeatureDeinitialize instead!
	 ********************************************************************************************* */
public:
	/** Overridden and marked as final to prevent their overriding in child classes, use OnGameFeatureInitialize/OnGameFeatureDeinitialize instead! */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override final;
	virtual void Deinitialize() override final { Super::Deinitialize(); }
	virtual void OnWorldBeginPlay(UWorld& InWorld) override final;
	virtual void OnWorldEndPlay(UWorld& InWorld) override final;
	virtual void OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override final;
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override final;

	/** Overridden and marked as final to prevet this subsystem from being created outside a game feature plugin context. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override final;

	/** Overridden and marked as final to prevent their overriding in child classes, use IsTickable() and Tick() instead if per-frame updates are needed. */
	virtual TStatId GetStatId() const override final { RETURN_QUICK_DECLARE_CYCLE_STAT(ThisClass, STATGROUP_Tickables); }
};