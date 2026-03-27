// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "BmrGeneratedMapSubsystem.generated.h"

class ABmrGeneratedMap;

/**
 * Provides access to the Generated Map and its world from anywhere in game as well as in editor.
 */
UCLASS()
class BOMBER_API UBmrGeneratedMapSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns the Generated Map Subsystem, is checked and wil crash if can't be obtained. */
	static UBmrGeneratedMapSubsystem& Get(const UObject* WorldContextObject = nullptr);

	/** Returns the pointer to the Generated Map Subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "WorldContextObject"))
	static UBmrGeneratedMapSubsystem* GetGeneratedMapSubsystem(const UObject* WorldContextObject = nullptr);

	/*********************************************************************************************
	 * Readiness
	 ********************************************************************************************* */
public:
	/** Returns true the Generated Map actor is ready to generate actors.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsGeneratedMapReady() const { return GeneratedMap && bAreDataAssetsLoaded; }

protected:
	/** Broadcasts GeneratedMap_Ready event if both the Generated Map actor and dependent actors' data assets are available */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TryBroadcastGeneratedMapReady();

	/*********************************************************************************************
	 * Generated Map
	 ********************************************************************************************* */
public:
	/** The Generated Map getter, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Level"))
	ABmrGeneratedMap* GetGeneratedMap(bool bWarnIfNull = true) const;

	/** The Generated Map setter */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetGeneratedMap(ABmrGeneratedMap* InGeneratedMap);

protected:
	/** Is main game actor on persistent level.
	 * @see UGeneratedMapSubsystem::GetGeneratedMap */
	UPROPERTY(Transient)
	TObjectPtr<ABmrGeneratedMap> GeneratedMap = nullptr;

	/** Is set to true when all required data assets (level actor + generated map) are loaded */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "[Bomber]")
	bool bAreDataAssetsLoaded = false;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called on subsystem creation, used for listening the readiness of the Generated Map and its data assets. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};