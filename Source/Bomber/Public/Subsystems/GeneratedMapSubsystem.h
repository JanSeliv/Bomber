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
#pragma region GetGeneratedMapSubsystem
	/** Returns the Generated Map Subsystem, is checked and wil crash if can't be obtained. */
	static UGeneratedMapSubsystem& Get();

	/** Returns the pointer to the Generated Map Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static UGeneratedMapSubsystem* GetGeneratedMapSubsystem(const UObject* WorldContextObject = nullptr);
#pragma endregion GetGeneratedMapSubsystem

	/** The Generated Map getter, nullptr otherwise */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Level"))
	AGeneratedMap* GetGeneratedMap() const;

	/** The Generated Map setter. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetGeneratedMap(AGeneratedMap* InGeneratedMap);

private:
	/** Is main game actor on persistent level.
	 * @see UGeneratedMapSubsystem::GetGeneratedMap */
	UPROPERTY(Transient)
	TObjectPtr<AGeneratedMap> GeneratedMapInternal = nullptr;
};
