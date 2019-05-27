// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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


	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "C++")
		static FORCEINLINE float GetVectorsLength(FVector x, FVector y)
	{
		return FGenericPlatformMath::Abs((x - y).Size());
	}


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
		static FORCEINLINE class AGeneratedMap* const GetLevelMap()
	{
		return GetSingleton()->levelMap;
	}

	class AGeneratedMap* levelMap;

};
