// Copyright 2021 Yevhenii Selivanov.

#include "GameFramework/MyGameModeBase.h"
//---
#include "GeneratedMap.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/SingletonLibrary.h"
#include "UI/MyHUD.h"

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

	// Set authority level map
	if (AGeneratedMap* LevelMap = USingletonLibrary::FindLevelMap(this))
	{
		LevelMap->SetOwner(NewPlayer);
		USingletonLibrary::SetLevelMap(LevelMap);
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
