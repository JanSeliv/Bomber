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
		static class AGeneratedMap* const GetGeneratedMap();

	static class AGeneratedMap* levelMap;

};
