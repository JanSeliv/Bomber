// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "BmrModularGameFeaturesSubsystem.generated.h"

/**
 * Manages tag-driven Modular Game Feature loading/unloading at runtime.
 * Features that should always be loaded must set their BuiltInInitialFeatureState to Active in .uplugin instead.
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
	/** Subscribes to GeneratedMap_Ready for tag-driven features. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Unloads all managed features on world end play. */
	virtual void OnWorldEndPlay(UWorld& InWorld) override;
};