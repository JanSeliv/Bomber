// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SingletonLibrary.generated.h"

UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USingletonLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	USingletonLibrary();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static USingletonLibrary* const GetSingleton();

	/* Cell funs */
	// Creates a cell from the vector
	UFUNCTION(BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "toCell"))
	static const FORCEINLINE FCell MakeCell(const AActor* actor)
	{
		return FCell(actor);
	}

	// Bound of floor
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get Grid Size"))
	static const FORCEINLINE float GetFloorLength()
	{
		return 200.0;
	}

	// The length between two cells
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "C++")
	static const FORCEINLINE float CalculateCellsLength(const FCell& x, const FCell& y)
	{
		return (fabsf((x.location - y.location).Size()) / GetFloorLength());
	}

	/* GeneratedMap funs*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static class AGeneratedMap* const GetLevelMap(UObject* WorldContextObject);

#if WITH_EDITORONLY_DATA
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateAiWithRenderParam);

	FUpdateAiWithRenderParam OnRenderAiUpdatedDelegate;
#endif

private:
	class AGeneratedMap* levelMap_;
	friend class AGeneratedMap;
};
