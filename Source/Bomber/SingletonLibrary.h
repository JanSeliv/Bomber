// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SingletonLibrary.generated.h"

UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USingletonLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	USingletonLibrary();

	/** 
	 * The singleton getter
	 * @return The singleton, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static USingletonLibrary* const GetSingleton();

	/** @addtogroup cell_functions
	 * The custom make node of the FCell struct that used instead of the default MakeStruct node
	 * @param Actor Finding the closest cell by actor
	 * @return The found cell
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "toCell"))
	static const FORCEINLINE FCell MakeCell(const AActor* Actor)
	{
		return FCell(Actor);
	}

	/** @addtogroup cell_functions
	 * @return The length of one cell (a floor bound)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get Grid Size"))
	static const FORCEINLINE float GetFloorLength()
	{
		return 200.0;
	}

	/** @addtogroup cell_functions
	 * Calculate the length between two cells
	 * @param X The first cell
	 * @param Y The other one cell
	 * @return The distance between to cells
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "C++")
	static const FORCEINLINE float CalculateCellsLength(const FCell& X, const FCell& Y)
	{
		return (fabsf((X.Location - Y.Location).Size()) / GetFloorLength());
	}

	/* The Level Map getter */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static class AGeneratedMap* const GetLevelMap(UObject* WorldContextObject);

#if WITH_EDITORONLY_DATA
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPushNongeneratedToMap);
	/** 
	 * Owners Map Components binds to updating on the Level Map to this delegate
	 * The Level Map broadcasts this delegate after own generation
	 * @see class UMapComponent
	 * @warning PIE only
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "C++")
	FPushNongeneratedToMap OnActorsUpdatedDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateAiWithRenderParam);
	/**
	 * Dynamic update visualization(text renders) of the bot's movements in the editor
	 * Debug AI-bot with bShouldShowRenders param binds to this delegate
	 * Map components owners broadcast this delegate
	 * on destroying or OnActorsUpdatedDelegate broadcasting
	 * @warning PIE only
	 */
	UPROPERTY()
	FUpdateAiWithRenderParam OnRenderAiUpdatedDelegate;
#endif

private:
	/** The reference to the AGeneratedMap actor*/
	class AGeneratedMap* LevelMap_;

	/** Access to the Level Map to keep an in gaming valid reference */
	friend class AGeneratedMap;
};
