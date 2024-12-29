// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "AIUtilsLibrary.generated.h"

/**
 * The cinematic functions library.
 * Extends Epic's API with some useful functions and tricks.
 */
UCLASS()
class MYUTILS_API UAIUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Finds and transform Navigation Mesh. */  
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Transform"))
	static void RebuildNavMesh(UObject* WorldContextObject, const FTransform& Transform);

	/** Returns true if the Navigation Mesh can be rebuilt. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static bool CanRebuildNavMesh(const class UNavigationSystemV1* NavSys);

	/** Returns true if the AI Controller is running any behavior tree. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsRunningAnyBehaviorTree(const class AAIController* AIController);
};
