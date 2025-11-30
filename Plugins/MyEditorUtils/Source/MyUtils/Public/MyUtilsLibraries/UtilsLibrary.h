// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UtilsLibrary.generated.h"

enum EAspectRatioAxisConstraint : int;

/**
 * The common functions library
 */
UCLASS()
class MYUTILS_API UUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*********************************************************************************************
	 * Play In Editor
	 ********************************************************************************************* */
public:
	/** Returns the current play world.
	 * Will attempt to get world from World Context Object if specified. */
	static UWorld* GetPlayWorld(const UObject* OptionalWorldContext = nullptr);

	/** Checks, is the current world placed in the editor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditor();

	/** Checks is the current world placed in the editor and the game not started yet. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorNotPieWorld();

	/** Returns true if game is started in the Editor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsPIE();

	/** Returns true if game was started. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool HasWorldBegunPlay();

	/*********************************************************************************************
	 * Viewport
	 ********************************************************************************************* */
public:
	/** Returns current viewport. */
	static const class FViewport* GetViewport();

	/** Returns true if viewport is initialized, is always true in PIE, but takes a while in builds. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsViewportInitialized();

	/** Returns the actual screen resolution.
	 * Is most reliable in comparisons with other ways get resolution like settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FIntPoint GetViewportResolution();

	/** Returns 'MaintainYFOV' if Horizontal FOV is currently used while 'MaintainXFOV' for the Vertical one.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static TEnumAsByte<EAspectRatioAxisConstraint> GetViewportAspectRatioAxisConstraint();

	/** Returns true if the current viewport is focused. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsViewportFocused();
};
