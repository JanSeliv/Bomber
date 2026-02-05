// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/BmrMVVM_GameViewModel.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Components/BmrMouseActivityComponent.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "DataAssets/BmrUIDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMVVM_GameViewModel)

/*********************************************************************************************
 * Current Game State
 ********************************************************************************************* */

// Called when the current game state was changed
void UBmrMVVM_GameViewModel::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const EBmrCurrentGameState InGameState = ABmrGameState::GetCurrentGameState();
	SetCurrentGameState(InGameState);

	SetCanRestartGame(CanStartGame());

	StopInGameCountdown();
	StopStartingCountdown();

	switch (InGameState)
	{
		case ECGS::GameStarting:
			TriggerStartingCountdown();
			break;
		case ECGS::InGame:
			TriggerInGameCountdown();
			break;
		default:
			break;
	}
}

/*********************************************************************************************
 * Can Restart Game
 ********************************************************************************************* */

// Returns true if the match can be started or restarted
bool UBmrMVVM_GameViewModel::CanStartGame() const
{
	const EBmrCurrentGameState GameState = ABmrGameState::GetCurrentGameState();

	if (GameState == ECGS::GameStarting)
	{
		// The game is already starting (3-2-1)
		return false;
	}

	if (UMultiplayerUtilsLibrary::IsMultiplayerGame())
	{
		// In multiplayer, the game can be started only when it is ended or not running yet
		return GameState == EBmrCurrentGameState::EndGame
		       || GameState == EBmrCurrentGameState::Menu;
	}

	// In singleplayer, the game can be started or restarted at any time
	return true;
}

/*********************************************************************************************
 * End-Game State
 ********************************************************************************************* */

// Called when the player state was changed
void UBmrMVVM_GameViewModel::OnEndGameStateChanged_Implementation(EBmrEndGameState NewEndGameState)
{
	const ESlateVisibility NewVisibility = NewEndGameState == EBmrEndGameState::None ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	SetEndGameStateVisibility(NewVisibility);

	SetEndGameResult(UBmrUIDataAsset::Get().GetEndGameText(NewEndGameState));
}

/*********************************************************************************************
 * Countdown timers
 ********************************************************************************************* */

// Starts counting the 3-2-1-GO timer when match is starting
void UBmrMVVM_GameViewModel::TriggerStartingCountdown()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 StartingCountdown = UBmrGameStateDataAsset::Get().GetStartingCountdown();
	SetStartingTimerSecRemain(FText::AsNumber(StartingCountdown));

	constexpr bool bInLoop = true;
	World->GetTimerManager().SetTimer(StartingTimer, this, &ThisClass::OnStartingTimerTick, DefaultTimerIntervalSec, bInLoop);
}

// Clears the Starting timer and stops counting it
void UBmrMVVM_GameViewModel::StopStartingCountdown()
{
	if (StartingTimer.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StartingTimer);
		}
	}
}

// Is called once a second during the Game Starting state to decrement the 'Three-two-one-GO' timer
void UBmrMVVM_GameViewModel::OnStartingTimerTick()
{
	const int32 CurrentValue = FCString::Atoi(*StartingTimerSecRemain.ToString());
	const int32 NewValue = CurrentValue - 1;
	SetStartingTimerSecRemain(FText::AsNumber(NewValue));

	if (NewValue <= 0)
	{
		StopStartingCountdown();
	}
}

// Starts counting the (120...0) timer during the match
void UBmrMVVM_GameViewModel::TriggerInGameCountdown()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 InGameCountdown = UBmrGameStateDataAsset::Get().GetInGameCountdown();
	SetInGameTimerSecRemain(FText::AsNumber(InGameCountdown));

	constexpr bool bInLoop = true;
	World->GetTimerManager().SetTimer(InGameTimer, this, &ThisClass::OnInGameTimerTick, DefaultTimerIntervalSec, bInLoop);
}

// Clears the In-Game timer and stops counting it
void UBmrMVVM_GameViewModel::StopInGameCountdown()
{
	if (InGameTimer.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InGameTimer);
		}
	}
}

// Is called once a second during the In-Game state to decrement the match timer
void UBmrMVVM_GameViewModel::OnInGameTimerTick()
{
	const int32 CurrentValue = FCString::Atoi(*InGameTimerSecRemain.ToString());
	const int32 NewValue = CurrentValue - 1;
	SetInGameTimerSecRemain(FText::AsNumber(NewValue));

	if (NewValue <= 0)
	{
		StopInGameCountdown();
	}
}

/*********************************************************************************************
 * Mouse Visibility
 ********************************************************************************************* */

// Called when mouse became shown or hidden
void UBmrMVVM_GameViewModel::OnMouseVisibilityChanged_Implementation(bool bIsShown)
{
	const ESlateVisibility NewVisibility = bIsShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	SetMouseVisibility(NewVisibility);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UBmrMVVM_GameViewModel::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
}

// Is called when this View Model is destructed
void UBmrMVVM_GameViewModel::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	StopStartingCountdown();
	StopInGameCountdown();
}

// Called when the local player character is spawned, possessed, and replicated
void UBmrMVVM_GameViewModel::OnLocalPawnReady_Implementation(const FGameplayEventData& Payload)
{
	const ABmrPawn* Pawn = Cast<ABmrPawn>(Payload.Instigator.Get());
	ABmrPlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<ABmrPlayerState>() : nullptr;
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);

	UBmrMouseActivityComponent* MouseActivityComponent = UBmrBlueprintFunctionLibrary::GetMouseActivityComponent();
	checkf(MouseActivityComponent, TEXT("ERROR: [%i] %hs:\n'MouseActivityComponent' is null!"), __LINE__, __FUNCTION__);
	MouseActivityComponent->OnMouseVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnMouseVisibilityChanged);
}