// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/GameSession.h"
//---
#include "MyGameSession.generated.h"

/**
 * Holds server-only data about this session like Session Name etc.
 */
UCLASS()
class BOMBER_API AMyGameSession : public AGameSession
{
	GENERATED_BODY()

protected:
	/** Is overridden to set proper ID for the player. */
	virtual void RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite) override;
};