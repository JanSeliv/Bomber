// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MultiplayerUtilsLibrary)

// Returns amount of players (host + clients) playing this game
int32 UMultiplayerUtilsLibrary::GetPlayersInMultiplayerNum()
{
	int32 PlayersNum = 0;

	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return PlayersNum;
	}

	for (const APlayerState* PlayerStateIt : GameState->PlayerArray)
	{
		if (PlayerStateIt && !PlayerStateIt->IsABot())
		{
			++PlayersNum;
		}
	}

	return PlayersNum;
}

// Returns the ping to the server in milliseconds
float UMultiplayerUtilsLibrary::GetPingMs()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	const UNetConnection* ServerConnection = NetDriver ? NetDriver->ServerConnection : nullptr;
	const APlayerController* PlayerController = ServerConnection ? ServerConnection->PlayerController : nullptr;
	const APlayerState* LocalPlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	return LocalPlayerState ? LocalPlayerState->GetPingInMilliseconds() : 0.f;
}
