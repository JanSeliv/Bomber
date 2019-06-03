// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GeneratedMap.h"
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
		static FORCEINLINE FCell MakeCell(const AActor* actor)
	{
		return FCell(actor);
	}

	// Bound of floor
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
		static FORCEINLINE float GetFloorLength()
	{
		return 200.0;
	}

	// The length between two cells
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "C++")
		static FORCEINLINE float CalculateCellsLength(const FCell& x, const FCell& y)
	{
		return (fabsf((x.location - y.location).Size()) / GetFloorLength());
	}


	/* GeneratedMap funs*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
		static FORCEINLINE class AGeneratedMap* const GetLevelMap()
	{
		return (IsValid(GetSingleton()) ? GetSingleton()->levelMap_ : nullptr);
	}




	// All used blueprints
	UPROPERTY(BlueprintReadWrite, Category = "C++", meta = (BlueprintBaseOnly))
		TArray<TSubclassOf<AActor>> bpClasses;

protected:
	friend class AGeneratedMap;
	class AGeneratedMap* levelMap_;
};
