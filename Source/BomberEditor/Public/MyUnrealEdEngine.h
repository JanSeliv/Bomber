// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Editor/UnrealEdEngine.h"
//---
#include "MyUnrealEdEngine.generated.h"

/**
 * Extends the Unreal Editor Engine class
 * to provide own singleton objects for editor clients in multiplayer.
 */
UCLASS(Transient)
class BOMBEREDITOR_API UMyUnrealEdEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnAnyDataAssetChanged);
	/** Will notify on any data asset changes. */
	static FOnAnyDataAssetChanged GOnAnyDataAssetChanged;

	DECLARE_MULTICAST_DELEGATE(FUpdateAI);
	/** Binds to update movements of each AI controller. */
	static FUpdateAI GOnAIUpdatedDelegate;

	/** Returns this Unreal Editor Engine object. */
	static const UMyUnrealEdEngine& Get();
};
