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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratedMapReady, class ABmrGeneratedMap*, GeneratedMap);

	/** Called when Generated Map is initialized and ready to be used, is also called in editor. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnGeneratedMapReady OnGeneratedMapReady;

	/** Returns true if level has generated map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool HasGeneratedMap() const { return GeneratedMap != nullptr; }

	/** The Generated Map getter, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Level"))
	ABmrGeneratedMap* GetGeneratedMap(bool bWarnIfNull = true) const;

	/** The Generated Map setter. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetGeneratedMap(ABmrGeneratedMap* InGeneratedMap);

private:
	/** Is main game actor on persistent level.
	 * @see UGeneratedMapSubsystem::GetGeneratedMap */
	UPROPERTY(Transient)
	TObjectPtr<ABmrGeneratedMap> GeneratedMap = nullptr;
};