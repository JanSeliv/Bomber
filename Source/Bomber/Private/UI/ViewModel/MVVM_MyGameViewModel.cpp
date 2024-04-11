// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyGameViewModel.h"
//---
#include "DataAssets/UIDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyGameViewModel)

/*********************************************************************************************
 * End-Game State
 ********************************************************************************************* */

// Called when the player state was changed
void UMVVM_MyGameViewModel::OnEndGameStateChanged(EEndGameState NewEndGameState)
{
	const ESlateVisibility NewVisibility = NewEndGameState == EEndGameState::None ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	SetEndGameStateVisibility(NewVisibility);

	SetEndGameResult(UUIDataAsset::Get().GetEndGameText(NewEndGameState));
}

/*********************************************************************************************
 * Countdown timers
 ********************************************************************************************* */

// Called when the 'Three-two-one-GO' timer was updated
void UMVVM_MyGameViewModel::OnStartingTimerSecRemainChanged_Implementation(float NewStartingTimerSecRemain)
{
	const int32 Value = FMath::CeilToInt(NewStartingTimerSecRemain);
	SetStartingTimerSecRemain(FText::AsNumber(Value));
}

// Called when remain seconds to the end of the match timer was updated
void UMVVM_MyGameViewModel::OnInGameTimerSecRemainChanged_Implementation(float NewInGameTimerSecRemain)
{
	const int32 Value = FMath::CeilToInt(NewInGameTimerSecRemain);
	SetInGameTimerSecRemain(FText::AsNumber(Value));
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UMVVM_MyGameViewModel::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	UGlobalEventsSubsystem& GlobalEventsSubsystem = UGlobalEventsSubsystem::Get();
	GlobalEventsSubsystem.OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::SetCurrentGameState);
	GlobalEventsSubsystem.OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);

	if (AMyGameStateBase* GameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		OnGameStateCreated(GameState);
	}
	else
	{
		UWorld* World = GetWorld();
		checkf(World, TEXT("ERROR: [%i] %s:\n'World' is null!"), __LINE__, *FString(__FUNCTION__));
		World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateCreated);
	}
}

// Is called when this View Model is destructed
void UMVVM_MyGameViewModel::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (UGlobalEventsSubsystem* GlobalEventsSubsystem = UGlobalEventsSubsystem::GetGlobalEventsSubsystem())
	{
		GlobalEventsSubsystem->OnGameStateChanged.RemoveAll(this);
		GlobalEventsSubsystem->OnEndGameStateChanged.RemoveAll(this);
	}

	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnStartingTimerSecRemainChanged.RemoveAll(this);
		MyGameState->OnInGameTimerSecRemainChanged.RemoveAll(this);
	}
}

// Called when Game State was created in current world
void UMVVM_MyGameViewModel::OnGameStateCreated_Implementation(AGameStateBase* GameState)
{
	AMyGameStateBase& MyGameState = *CastChecked<AMyGameStateBase>(GameState);
	MyGameState.OnStartingTimerSecRemainChanged.AddUniqueDynamic(this, &ThisClass::OnStartingTimerSecRemainChanged);
	MyGameState.OnInGameTimerSecRemainChanged.AddUniqueDynamic(this, &ThisClass::OnInGameTimerSecRemainChanged);
}