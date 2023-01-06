// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "UtilsLibrary.generated.h"

class UWidget;

/**
 * The common functions library
 */
UCLASS()
class MYUTILS_API UUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if viewport is initialized, is always true in PIE, but takes a while in builds. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsViewportInitialized();
};
