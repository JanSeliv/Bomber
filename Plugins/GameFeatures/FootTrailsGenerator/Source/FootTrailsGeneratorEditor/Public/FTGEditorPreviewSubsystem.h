// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EditorSubsystem.h"

// UE
#include "GameFeatureStateChangeObserver.h"

#include "FTGEditorPreviewSubsystem.generated.h"

/**
 * Handles foot trails automatic previewing in Editor before game starts.
 * Listens to its own Modular Game Feature plugin state to gate the preview lifecycle.
 */
UCLASS()
class FOOTTRAILSGENERATOREDITOR_API UFTGEditorPreviewSubsystem : public UEditorSubsystem
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Runtime component to generate preview of foot trails. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[FootTrailsGenerator]", meta = (BlueprintProtected))
	TObjectPtr<class UFTGComponent> FootTrailGenerator = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called when the subsystem is initialized. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Is called when the subsystem is deinitialized. */
	virtual void Deinitialize() override;

	/** Is called when any game feature plugin activates, filtered to the owning plugin to start previewing. */
	virtual void OnGameFeatureActivating(const class UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/** Is called when any game feature plugin deactivates, filtered to the owning plugin to tear down the preview. */
	virtual void OnGameFeatureDeactivating(const class UGameFeatureData* GameFeatureData, struct FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override;

	/** Subscribes to Generated Map readiness once the owning plugin becomes active. */
	void OnGameFeatureInitialize();

	/** Destroys any spawned preview component and unsubscribes once the owning plugin becomes inactive. */
	void OnGameFeatureDeinitialize();

	/** Called when Generated Map is initialized and its data assets are loaded, is also called in editor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[FootTrailsGenerator]", meta = (BlueprintProtected))
	void OnGeneratedMapReady(const struct FGameplayEventData& Payload);
};
