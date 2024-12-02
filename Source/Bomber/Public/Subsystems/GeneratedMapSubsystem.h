// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "GeneratedMapSubsystem.generated.h"

class AGeneratedMap;

/**
 * Provides access to the Generated Map and its world from anywhere in game as well as in editor.
 */
UCLASS()
class BOMBER_API UGeneratedMapSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns the Generated Map Subsystem, is checked and wil crash if can't be obtained. */
	static UGeneratedMapSubsystem& Get(const UObject* WorldContextObject = nullptr);

	/** Returns the pointer to the Generated Map Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static UGeneratedMapSubsystem* GetGeneratedMapSubsystem(const UObject* WorldContextObject = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratedMapReady, class AGeneratedMap*, GeneratedMap);

	/** Called when Generated Map is initialized and ready to be used, is also called in editor. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGeneratedMapReady OnGeneratedMapReady;

	/** Returns true if level has generated map. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool HasGeneratedMap() const { return GeneratedMapInternal != nullptr; }

	/** The Generated Map getter, nullptr otherwise */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Level"))
	AGeneratedMap* GetGeneratedMap(bool bWarnIfNull = true) const;

	/** The Generated Map setter. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetGeneratedMap(AGeneratedMap* InGeneratedMap);

private:
	/** Is main game actor on persistent level.
	 * @see UGeneratedMapSubsystem::GetGeneratedMap */
	UPROPERTY(Transient)
	TObjectPtr<AGeneratedMap> GeneratedMapInternal = nullptr;
};