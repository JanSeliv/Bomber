// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"

/** __CALLER__ displays previous function in callstack, can be displayed in log by %hs. e.g:
 * UE_LOG(LogTemp, Log, TEXT("%hs"), __CALLER__); */
#define __CALLER__ FDebugUtilsLibrary::GetCallerFunctionANSI()

/**
 * Contains debug utilities for non-shipping game.
 * Is useful for debugging and profiling. 
 */
class MYUTILS_API FDebugUtilsLibrary
{
public:
	/** Returns previous function in current callstack.
	 * Can be passed in log by as %hs as __CALLER__ */
	static const ANSICHAR* GetCallerFunctionANSI();
};
