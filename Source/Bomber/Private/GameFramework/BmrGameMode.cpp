// Copyright (c) Yevhenii Selivanov.

#include "GameFramework/BmrGameMode.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "GameFramework/BmrGameSession.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameMode)

// Sets default values for this actor's properties
ABmrGameMode::ABmrGameMode()
{
	// Custom default classes. All of them can be overridden in child game mode in World Settings
	GameStateClass = ABmrGameState::StaticClass();
	PlayerControllerClass = ABmrPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = ABmrPlayerController::StaticClass();
	PlayerStateClass = ABmrPlayerState::StaticClass();
	GameSessionClass = ABmrGameSession::StaticClass();

	// Spawn and possess pawn by ourselves manually
	DefaultPawnClass = nullptr;

	bUseSeamlessTravel = true;
}

// Returns player controller by specified index
ABmrPlayerController* ABmrGameMode::GetPlayerController(int32 Index) const
{
	if (PlayerControllers.IsValidIndex(Index))
	{
		return PlayerControllers[Index];
	}
	return nullptr;
}

// Caches given player controller when it spawns
void ABmrGameMode::AddPlayerController(ABmrPlayerController* PlayerController)
{
	PlayerControllers.Add(PlayerController);
}

// Initializes the game
void ABmrGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
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
void ABmrGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const ABmrPlayerController* MyPC = Cast<ABmrPlayerController>(NewPlayer);
	if (!MyPC)
	{
		return;
	}

	// When new player joins to running game (any state is set), return everyone to the menu: joining running match and spectating are not supported at this moment
	if (ABmrGameState::GetCurrentGameState() != ECGS::None)
	{
		FGameplayEventData EventData;
		EventData.EventTag = BmrGameplayTags::Event::Player_PostLogin;
		EventData.Instigator = NewPlayer;
		UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);
	}

	if (APlayerState* PlayerState = MyPC->GetPlayerState<APlayerState>())
	{
		// Disabled to allow start the game in 'Simulate in Editor' (F6) and then press 'Possess or eject player' button
		PlayerState->SetIsOnlyASpectator(false);
	}
}

// Called when a Controller with a PlayerState leaves the game or is destroyed
void ABmrGameMode::Logout(AController* Exiting)
{
	ABmrPlayerController* MyPC = Cast<ABmrPlayerController>(Exiting);
	if (MyPC && MyPC->HasClientLoadedCurrentWorld())
	{
		PlayerControllers.RemoveSwap(MyPC);
	}

	Super::Logout(Exiting);
}

// Sets the name for a controller
void ABmrGameMode::ChangeName(AController* Controller, const FString& NewName, bool bNameChange)
{
	// Super is not called since it's forbidden to change player name with this function
	// Instead, ABmrPlayerState API should be used
}

#if WITH_EDITOR
// Is called if start the game in 'Simulate in Editor' and then press 'Possess or eject player' button
bool ABmrGameMode::SpawnPlayerFromSimulate(const FVector& NewLocation, const FRotator& NewRotation)
{
	// Super is not called since there no need to spawn default player
	return true;
}
#endif // WITH_EDITOR