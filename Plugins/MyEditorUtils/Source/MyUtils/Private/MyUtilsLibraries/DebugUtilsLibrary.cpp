// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/DebugUtilsLibrary.h"
//---
#if !UE_BUILD_SHIPPING
#include "HAL/PlatformStackWalk.h"
#endif // !UE_BUILD_SHIPPING

// Returns previous function in current callstack
const ANSICHAR* FDebugUtilsLibrary::GetCallerFunctionANSI()
{
#if UE_BUILD_SHIPPING
	static constexpr ANSICHAR UnavailableMsg[] = "None";
	return UnavailableMsg;
#else
	constexpr int32 MaxDepth = 10;
	constexpr int32 CallerFrameIndex = 3;

	static ANSICHAR FunctionName[1024] = "Unknown caller";
	uint64 StackTrace[MaxDepth] = {0};

	const int32 Depth = FPlatformStackWalk::CaptureStackBackTrace(StackTrace, MaxDepth);
	if (Depth <= CallerFrameIndex)
	{
		return FunctionName;
	}

	FProgramCounterSymbolInfo SymbolInfo;
	FPlatformStackWalk::ProgramCounterToSymbolInfo(StackTrace[CallerFrameIndex], /*out*/SymbolInfo);
	if (SymbolInfo.FunctionName[0] == '\0')
	{
		return FunctionName;
	}

	FCStringAnsi::Strncpy(FunctionName, SymbolInfo.FunctionName, sizeof(FunctionName) - 1);
	FunctionName[sizeof(FunctionName) - 1] = '\0';

	if (ANSICHAR* BracketPos = FCStringAnsi::Strrchr(FunctionName, '('))
	{
		*BracketPos = '\0';
	}

	return FunctionName;
#endif // !UE_BUILD_SHIPPING
}
