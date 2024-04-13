// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyGameViewModel.h"
//---
#include "DataAssets/ItemDataAsset.h"
#include "DataAssets/UIDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
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
void UMVVM_MyGameViewModel::OnEndGameStateChanged_Implementation(EEndGameState NewEndGameState)
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
 * PowerUps
 ********************************************************************************************* */

// Called when power-ups were updated on local character
void UMVVM_MyGameViewModel::OnPowerUpsChanged_Implementation(const FPowerUp& NewPowerUps)
{
	SetPowerUpSkateN(FText::AsNumber(NewPowerUps.SkateN));
	SetPowerUpBombN(FText::AsNumber(NewPowerUps.BombN));
	SetPowerUpFireN(FText::AsNumber(NewPowerUps.FireN));

	const float MaxPowerUps = static_cast<float>(UItemDataAsset::Get().GetMaxAllowedItemsNum());
	checkf(MaxPowerUps > 0.f, TEXT("ERROR: [%i] %s:\n'MaxPowerUps > 0' is null!"), __LINE__, *FString(__FUNCTION__));
	SetPowerUpSkatePercent(static_cast<float>(NewPowerUps.SkateN) / MaxPowerUps);
	SetPowerUpBombPercent(static_cast<float>(NewPowerUps.BombN) / MaxPowerUps);
	SetPowerUpFirePercent(static_cast<float>(NewPowerUps.FireN) / MaxPowerUps);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UMVVM_MyGameViewModel::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	BIND_AND_CALL_ON_GAME_STATE_CHANGED(this, ThisClass::SetCurrentGameState);

	BIND_AND_CALL_ON_CHARACTER_READY(this, ThisClass::OnCharacterReady, INDEX_NONE);

	BIND_ON_GAME_STATE_CREATED(this, ThisClass::OnGameStateCreated);
}

// Is called when this View Model is destructed
void UMVVM_MyGameViewModel::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (UGlobalEventsSubsystem* GlobalEventsSubsystem = UGlobalEventsSubsystem::GetGlobalEventsSubsystem())
	{
		GlobalEventsSubsystem->OnGameStateChanged.RemoveAll(this);}

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

// Called when local player character was possessed, so we can bind to data
void UMVVM_MyGameViewModel::OnCharacterReady_Implementation(APlayerCharacter* PlayerCharacter, int32 CharacterID)
{
	checkf(PlayerCharacter, TEXT("ERROR: [%i] %hs:\n'PlayerCharacter' is null!"), __LINE__, __FUNCTION__);
	if (!PlayerCharacter->IsLocallyControlled()
	    || !PlayerCharacter->IsPlayerControlled())
	{
		// This View Model is not for this character
		return;
	}

	PlayerCharacter->OnPowerUpsChanged.AddUniqueDynamic(this, &ThisClass::OnPowerUpsChanged);

	AMyPlayerState* PlayerState = PlayerCharacter->GetPlayerState<AMyPlayerState>();
	if (ensureAlwaysMsgf(PlayerState, TEXT("ASSERT: [%i] %s:\n'CharacterState' is null!"), __LINE__, *FString(__FUNCTION__)))
	{
		PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
	}
}