// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrGameSession.h"

// Bomber
#include "GameFramework/BmrPlayerState.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameSession)

// Is overridden to set proper ID for the player
void ABmrGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
	Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);

	ABmrPlayerState* PlayerState = NewPlayer ? NewPlayer->GetPlayerState<ABmrPlayerState>() : nullptr;
	if (PlayerState)
	{
		PlayerState->SetHumanId(NewPlayer);
	}
}