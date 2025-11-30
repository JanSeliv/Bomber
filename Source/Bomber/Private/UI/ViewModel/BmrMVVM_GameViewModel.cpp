// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/BmrMVVM_GameViewModel.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Components/BmrMouseActivityComponent.h"
#include "DataAssets/BmrUIDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMVVM_GameViewModel)

/*********************************************************************************************
 * Current Game State
 ********************************************************************************************* */

// Called when the current game state was changed
void UBmrMVVM_GameViewModel::OnGameStateChanged_Implementation(EBmrCurrentGameState InGameState)
{
	SetCurrentGameState(InGameState);

	SetCanRestartGame(ABmrGameState::Get().CanStartGame());
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

// Called when the 'Three-two-one-GO' timer was updated
void UBmrMVVM_GameViewModel::OnStartingTimerSecRemainChanged_Implementation(float NewStartingTimerSecRemain)
{
	const int32 Value = FMath::CeilToInt(NewStartingTimerSecRemain);
	SetStartingTimerSecRemain(FText::AsNumber(Value));
}

// Called when remain seconds to the end of the match timer was updated
void UBmrMVVM_GameViewModel::OnInGameTimerSecRemainChanged_Implementation(float NewInGameTimerSecRemain)
{
	const int32 Value = FMath::CeilToInt(NewInGameTimerSecRemain);
	SetInGameTimerSecRemain(FText::AsNumber(Value));
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

	BIND_ON_GAME_STATE_CREATED(this, ThisClass::OnGameStateCreated);
}

// Is called when this View Model is destructed
void UBmrMVVM_GameViewModel::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (UBmrGlobalEventsSubsystem* GlobalEventsSubsystem = UBmrGlobalEventsSubsystem::GetGlobalEventsSubsystem())
	{
		GlobalEventsSubsystem->BP_OnGameStateChanged.RemoveAll(this);
	}

	if (ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState())
	{
		MyGameState->OnStartingTimerSecRemainChanged.RemoveAll(this);
		MyGameState->OnInGameTimerSecRemainChanged.RemoveAll(this);
	}
}

// Called when Game State was created in current world
void UBmrMVVM_GameViewModel::OnGameStateCreated_Implementation(AGameStateBase* GameState)
{
	ABmrGameState& MyGameState = *CastChecked<ABmrGameState>(GameState);
	MyGameState.OnStartingTimerSecRemainChanged.AddUniqueDynamic(this, &ThisClass::OnStartingTimerSecRemainChanged);
	MyGameState.OnInGameTimerSecRemainChanged.AddUniqueDynamic(this, &ThisClass::OnInGameTimerSecRemainChanged);
	MyGameState.GetWorld()->GameStateSetEvent.RemoveAll(this);
}

// Called when the local player character is spawned, possessed, and replicated
void UBmrMVVM_GameViewModel::OnLocalPawnReady_Implementation(ABmrPawn* Pawn, int32 PlayerId)
{
	ABmrPlayerState* PlayerState = Pawn->GetPlayerState<ABmrPlayerState>();
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);

	UBmrMouseActivityComponent* MouseActivityComponent = UBmrBlueprintFunctionLibrary::GetMouseActivityComponent();
	checkf(MouseActivityComponent, TEXT("ERROR: [%i] %hs:\n'MouseActivityComponent' is null!"), __LINE__, __FUNCTION__);
	MouseActivityComponent->OnMouseVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnMouseVisibilityChanged);
}