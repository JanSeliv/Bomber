// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SingletonLibrary.generated.h"

UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USingletonLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
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
#endif  // WITH_EDITORONLY_DATA

	/** @addtogroup AI
	 * Call all signed as bShouldShowRenders AI characters
	 * @param Owner The called owner
	 * @warning PIE only
	 * @warning Is not static for OnDestroyed binding
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void BroadcastAiUpdating(AActor* Owner);

	/**
	 * Debug visualization by text renders
	 * @warning PIE only
	 * @warning Is not static for Blueprint realization
	 * @todo Don't remove nameplate render of AI
     */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "C++", meta = (DevelopmentOnly, AdvancedDisplay = 2, AutoCreateRefTerm = "RenderText"))
	void AddDebugTextRenders(
		class AActor* Owner,										  ///<
		const TSet<struct FCell>& Cells,							  ///<
		struct FLinearColor TextColor,								  ///<
		float TextHeight,											  ///<
		float TextSize,												  ///<
		const FText& RenderText,									  ///<
		FVector CoordinatePosition,									  ///<
		bool bShouldClearChildRenders,								  ///<
		TArray<class UTextRenderComponent*>& OutTextRenderComponents  ///<
		) const;

	/** @addtogroup cell_functions
	 * The custom make node of the FCell struct that used as a blueprint implementation of the default MakeStruct node
	 * @param Actor Finding the closest cell by actor
	 * @return The found cell
	 * @warning Deprecated, temporary function
	 * @warning Is not static for Blueprint realization
	 * @todo Rewrite to C++ FCell()
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "toCell"))
	FORCEINLINE struct FCell MakeCell(const class AActor* Actor) const;

	/** 
	 * The singleton getter
	 * @return The singleton, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE USingletonLibrary* GetSingleton()
	{
		if (GEngine == nullptr) return nullptr;
		return Cast<USingletonLibrary>(GEngine->GameSingleton);
	}

	/** @addtogroup cell_functions
	 * @return The length of one cell (a floor bound)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get Grid Size"))
	static FORCEINLINE float GetFloorLength()
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
	static FORCEINLINE float CalculateCellsLength(const struct FCell& X, const struct FCell& Y)
	{
		return (fabsf((X.Location - Y.Location).Size()) / GetFloorLength());
	}

	/** The Level Map getter */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static class AGeneratedMap* const GetLevelMap(UObject* WorldContextObject);

protected:
	/** The reference to the AGeneratedMap actor*/
	UPROPERTY(BlueprintReadOnly, Category = "C++")
	class AGeneratedMap* LevelMap_;

	/** Access to the Level Map to keep an in gaming valid reference */
	friend class AGeneratedMap;
};
