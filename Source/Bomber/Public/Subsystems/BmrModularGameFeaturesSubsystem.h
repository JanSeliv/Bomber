// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "BmrModularGameFeaturesSubsystem.generated.h"

/**
 * Manages tag-driven Modular Game Feature loading/unloading.
 * Reacts to any gameplay tag change on GeneratedMap ASC and evaluates which features should be active.
 * Works in editor world: subsystem and GeneratedMap both exist before PIE.
 */
UCLASS()
class BOMBER_API UBmrModularGameFeaturesSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this subsystem, is checked and will crash if can't be obtained. */
	static UBmrModularGameFeaturesSubsystem& Get(const UObject* WorldContextObject = nullptr);

	/** Returns the pointer to this subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "WorldContextObject"))
	static UBmrModularGameFeaturesSubsystem* GetModularGameFeaturesSubsystem(const UObject* WorldContextObject = nullptr);

	/*********************************************************************************************
	 * Tag-Driven Features
	 ********************************************************************************************* */
protected:
	/** Called when the Generated Map is initialized and ready, subscribes to ASC tag events for tag-driven features. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGeneratedMapReady(class ABmrGeneratedMap* GeneratedMap);

	/** Is called when any of the tag-driven features tags is added or removed from the Generated Map ASC, evaluates all tag-driven features and loads/unloads them accordingly. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnModularGameFeatureTagChanged();

	/** Pending next-frame evaluation handle, is valid when evaluation is already queued. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FTimerHandle EvaluationTimerHandle;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Binds to GeneratedMap readiness to subscribe to ASC tag events. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Loads default (non-tag-based) features when the world starts playing. */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Unloads default features on world end play. */
	virtual void OnWorldEndPlay(UWorld& InWorld) override;
};