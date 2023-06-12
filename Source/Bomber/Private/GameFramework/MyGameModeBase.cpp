// Copyright (c) Yevhenii Selivanov.

#include "GameFramework/MyGameModeBase.h"
//---
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "UI/MyHUD.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameModeBase)

// Sets default values for this actor's properties
AMyGameModeBase::AMyGameModeBase()
{
	// Custom defaults classes
	GameStateClass = AMyGameStateBase::StaticClass();
	HUDClass = AMyHUD::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	PlayerStateClass = AMyPlayerState::StaticClass();
}

// Returns player controller by specified index
AMyPlayerController* AMyGameModeBase::GetPlayerController(int32 Index) const
{
	if (PlayerControllersInternal.IsValidIndex(Index))
	{
		return PlayerControllersInternal[Index];
	}
	return nullptr;
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

	AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);
	if (!MyPC)
	{
		return;
	}

	if (APlayerState* PlayerState = MyPC->GetPlayerState<APlayerState>())
	{
		// Spectators are not supported
		PlayerState->SetIsOnlyASpectator(false);
	}

	PlayerControllersInternal.AddUnique(MyPC);
}

// Called when a Controller with a PlayerState leaves the game or is destroyed
void AMyGameModeBase::Logout(AController* Exiting)
{
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(Exiting);
	if (MyPC
	    && MyPC->HasClientLoadedCurrentWorld())
	{
		PlayerControllersInternal.Remove(MyPC);
	}

	Super::Logout(Exiting);
}

#if WITH_EDITOR
// Is called if start the game in 'Simulate in Editor' and then press 'Possess or eject player' button
bool AMyGameModeBase::SpawnPlayerFromSimulate(const FVector& NewLocation, const FRotator& NewRotation)
{
	// Super is not called since there no need to spawn default player
	return true;
}
#endif // WITH_EDITOR
