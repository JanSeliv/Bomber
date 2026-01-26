// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"

// MyUtils
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MultiplayerUtilsLibrary)

// Returns true if running from the server or party-leader client
bool UMultiplayerUtilsLibrary::IsPartyLeader()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	if (!World)
	{
		return false;
	}

	const bool bIsServer = World->GetNetMode() != NM_Client;
	const bool bIsAuthoritativeClient = IsAuthoritativeClient();
	return bIsServer || bIsAuthoritativeClient;
}

// Returns true in editor game is running from client-only mode, so if caller is client, which is connected to the dedicated server
bool UMultiplayerUtilsLibrary::IsAuthoritativeClient()
{
	if (!UUtilsLibrary::IsEditor())
	{
		// Client-only mode can run only from  the editor
		return false;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const bool bIsClient = World ? World->GetNetMode() == NM_Client : false;
	if (!bIsClient)
	{
		return false;
	}

	const bool bIsMultiplayerGame = IsMultiplayerGame();
	const bool bIsSinglePlayerClient = !bIsMultiplayerGame;
	const bool bIsPartyLeaderClient = bIsMultiplayerGame && GetPlayerId() == 0;
	return bIsSinglePlayerClient || bIsPartyLeaderClient;
}

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

// Returns the index of the current player during multiplayer
int32 UMultiplayerUtilsLibrary::GetPlayerId()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	const APlayerState* LocalPlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	return LocalPlayerState ? LocalPlayerState->GetPlayerId() : 0;
}

// Returns the ping to the server in milliseconds
float UMultiplayerUtilsLibrary::GetPingMs()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	const APlayerState* LocalPlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	return LocalPlayerState ? LocalPlayerState->GetPingInMilliseconds() : 0.f;
}

// Returns player's ping in seconds
float UMultiplayerUtilsLibrary::GetPlayerPingSeconds(const APawn* InPawn)
{
	const APlayerState* PlayerState = InPawn ? InPawn->GetPlayerState<APlayerState>() : nullptr;
	return PlayerState ? FTimespan::FromMilliseconds(PlayerState->GetPingInMilliseconds()).GetTotalSeconds() : 0.f;
}
