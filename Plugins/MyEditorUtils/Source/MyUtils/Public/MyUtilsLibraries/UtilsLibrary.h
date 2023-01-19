// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Engine/EngineTypes.h"
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
#pragma region Viewport
	/** Returns true if viewport is initialized, is always true in PIE, but takes a while in builds. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsViewportInitialized();

	/** Returns the actual screen resolution.
	 * Is most reliable in comparisons with other ways get resolution like settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FIntPoint GetViewportResolution();

	/** Returns 'MaintainYFOV' if Horizontal FOV is currently used while 'MaintainXFOV' for the Vertical one.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	static TEnumAsByte<EAspectRatioAxisConstraint> GetViewportAspectRatioAxisConstraint();
#pragma endregion Viewport
};
