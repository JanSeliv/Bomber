// Copyright (c) Yevhenii Selivanov

#include "BmrUnrealEdEngine.h"

// UE
#include "UnrealEdGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrUnrealEdEngine)

// Will notify on any data asset changes
UBmrUnrealEdEngine::FOnAnyDataAssetChanged UBmrUnrealEdEngine::GOnAnyDataAssetChanged;

// Binds to update movements of each AI controller.
UBmrUnrealEdEngine::FUpdateAI UBmrUnrealEdEngine::GOnAIUpdatedDelegate;

// Returns this Unreal Editor Engine object
const UBmrUnrealEdEngine& UBmrUnrealEdEngine::Get()
{
	const UBmrUnrealEdEngine* MyUnrealEdEngine = Cast<UBmrUnrealEdEngine>(GUnrealEd);
	checkf(MyUnrealEdEngine, TEXT("The My Unread Editor Engine is not valid"));
	return *MyUnrealEdEngine;
}
