// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyGameSession.h"
//---
#include "GameFramework/MyPlayerState.h"
//---
#include "GameFramework/PlayerController.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameSession)

// Is overridden to set proper ID for the player
void AMyGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
	Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);

	AMyPlayerState* PlayerState = NewPlayer ? NewPlayer->GetPlayerState<AMyPlayerState>() : nullptr;
	if (PlayerState)
	{
		PlayerState->SetHumanId(NewPlayer);
	}
}