// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/GameViewportClient.h"
//---
#include "MyGameViewportClient.generated.h"

/**
 * Is the engine's interface to a game viewport.
 * Implements parent to have more control on input events.
 */
UCLASS()
class BOMBER_API UMyGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	/** Gets whether or not the cursor should always be locked to the viewport. */
	virtual bool ShouldAlwaysLockMouse() override { return true; }
};
