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

// Caches given player controller when it spawns
void AMyGameModeBase::AddPlayerController(AMyPlayerController* PlayerController)
{
	PlayerControllersInternal.Add(PlayerController);
}

// Initializes the game
void AMyGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Override network version check, so cross-platform builds can connect to each other
	FNetworkVersion::IsNetworkCompatibleOverride.BindLambda([](uint32 LocalNetworkVersion, uint32 RemoteNetworkVersion)
	{
		// @TODO JanSeliv 0moajxBA: Generate game version on UI for builds, use it for builds compatibility check
		return true;
	});
}

// Called after a successful login
void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);
	if (!MyPC)
	{
		return;
	}

	if (APlayerState* PlayerState = MyPC->GetPlayerState<APlayerState>())
	{
		// Spectators are not supported
		PlayerState->SetIsOnlyASpectator(false);
	}
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

// Sets the name for a controller 
void AMyGameModeBase::ChangeName(AController* Controller, const FString& NewName, bool bNameChange)
{
	// Super is not called since it's forbidden to change player name with this function
	// Instead, AMyPlayerState API should be used
}

#if WITH_EDITOR
// Is called if start the game in 'Simulate in Editor' and then press 'Possess or eject player' button
bool AMyGameModeBase::SpawnPlayerFromSimulate(const FVector& NewLocation, const FRotator& NewRotation)
{
	// Super is not called since there no need to spawn default player
	return true;
}
#endif // WITH_EDITOR