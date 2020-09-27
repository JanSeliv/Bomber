// Copyright 2020 Yevhenii Selivanov.

#include "MyGameModeBase.h"
//---
#include "MyHUD.h"
#include "MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"

// Sets default values for this actor's properties
AMyGameModeBase::AMyGameModeBase()
{
	// Custom defaults classes
	GameStateClass = AMyGameStateBase::StaticClass();
	HUDClass = AMyHUD::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	PlayerStateClass = AMyPlayerState::StaticClass();
}

// Called when the game starts or when spawned
void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called after a successful login
void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	const int32 PlayersNum = GetNumPlayers();
	if (PlayersNum) // all players were connected
	{
		OnSessionStarted();
	}
}

// Called when all players were connected.
void AMyGameModeBase::OnSessionStarted() const
{

}
