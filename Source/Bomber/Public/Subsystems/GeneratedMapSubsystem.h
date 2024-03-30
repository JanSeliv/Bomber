// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"
//---
#include "GeneratedMapSubsystem.generated.h"

class AGeneratedMap;

/**
 * Provides access to the Generated Map from anywhere in game as well as in editor.
 */
UCLASS()
class BOMBER_API UGeneratedMapSubsystem final : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns the Generated Map Subsystem, is checked and will crash if can't be obtained. */
	static UGeneratedMapSubsystem& Get();

	/** Returns the pointer to the Generated Map Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static UGeneratedMapSubsystem* GetGeneratedMapSubsystem();

	/** The Generated Map getter, nullptr otherwise */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Level"))
	AGeneratedMap* GetGeneratedMap() const;

	/** The Generated Map setter. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (Keywords = "Level"))
	void SetGeneratedMap(AGeneratedMap* InGeneratedMap);

private:
	/** Is main game actor on persistent level.
	 * @see UGeneratedMapSubsystem::GetGeneratedMap */
	TWeakObjectPtr<AGeneratedMap> GeneratedMapInternal = nullptr;
};
