// Copyright (c) Yevhenii Selivanov

#include "MyUnrealEdEngine.h"
//---
#include "UnrealEdGlobals.h"

// Will notify on any data asset changes
UMyUnrealEdEngine::FOnAnyDataAssetChanged UMyUnrealEdEngine::GOnAnyDataAssetChanged;

// Binds to update movements of each AI controller.
UMyUnrealEdEngine::FUpdateAI UMyUnrealEdEngine::GOnAIUpdatedDelegate;

// Returns this Unreal Editor Engine object
const UMyUnrealEdEngine& UMyUnrealEdEngine::Get()
{
	const UMyUnrealEdEngine* MyUnrealEdEngine = Cast<UMyUnrealEdEngine>(GUnrealEd);
	checkf(MyUnrealEdEngine, TEXT("The My Unread Editor Engine is not valid"));
	return *MyUnrealEdEngine;
}
