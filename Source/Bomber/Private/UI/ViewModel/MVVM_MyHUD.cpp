// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyHUD.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyHUD)

/*********************************************************************************************
 * Countdown timers
 ********************************************************************************************* */

// Setter about the summary seconds of launching 'Three-two-one-GO' timer that is used on game starting
void UMVVM_MyHUD::SetStartingTimerSecRemain(const FText& NewStartingTimerSecRemain)
{
	UE_MVVM_SET_PROPERTY_VALUE(StartingTimerSecRemain, NewStartingTimerSecRemain);
}

// Setter about the seconds to the end of the round
void UMVVM_MyHUD::SetInGameTimerSecRemain(const FText& NewInGameTimerSecRemain)
{
	UE_MVVM_SET_PROPERTY_VALUE(InGameTimerSecRemain, NewInGameTimerSecRemain);
}

// Called when the 'Three-two-one-GO' timer was updated
void UMVVM_MyHUD::OnStartingTimerSecRemainChanged_Implementation(float NewStartingTimerSecRemain)
{
	const int32 Value = FMath::CeilToInt(NewStartingTimerSecRemain);
	SetStartingTimerSecRemain(FText::AsNumber(Value));
}

// Called when remain seconds to the end of the match timer was updated
void UMVVM_MyHUD::OnInGameTimerSecRemainChanged_Implementation(float NewInGameTimerSecRemain)
{
	const int32 Value = FMath::CeilToInt(NewInGameTimerSecRemain);
	SetInGameTimerSecRemain(FText::AsNumber(Value));
}

/*********************************************************************************************
 * PowerUps
 ********************************************************************************************* */

// Setter about current amount of speed power-ups
void UMVVM_MyHUD::SetPowerUpSkate(const FText& NewPowerUpSkate)
{
	UE_MVVM_SET_PROPERTY_VALUE(PowerUpSkate, NewPowerUpSkate);
}

// Setter about current amount of max bomb power-ups
void UMVVM_MyHUD::SetPowerUpBomb(const FText& NewPowerUpBomb)
{
	UE_MVVM_SET_PROPERTY_VALUE(PowerUpBomb, NewPowerUpBomb);
}

// Setter about current amount of blast radius power-ups
void UMVVM_MyHUD::SetPowerUpFire(const FText& NewPowerUpFire)
{
	UE_MVVM_SET_PROPERTY_VALUE(PowerUpFire, NewPowerUpFire);
}

// Called when power-ups were updated
void UMVVM_MyHUD::OnPowerUpsChanged_Implementation(const FPowerUp& NewPowerUps)
{
	SetPowerUpSkate(FText::AsNumber(NewPowerUps.SkateN));
	SetPowerUpBomb(FText::AsNumber(NewPowerUps.BombN));
	SetPowerUpFire(FText::AsNumber(NewPowerUps.FireN));
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UMVVM_MyHUD::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	// @TODO refactor in next commit: BIND_ON_CHARACTER_WITH_ID_POSSESSED(this, ThisClass::OnLocalPlayerReady);
}

// Is called when this View Model is destructed
void UMVVM_MyHUD::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnStartingTimerSecRemainChanged.RemoveAll(this);
		MyGameState->OnInGameTimerSecRemainChanged.RemoveAll(this);
	}

	if (APlayerCharacter* Player = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter())
	{
		Player->OnPowerUpsChanged.RemoveAll(this);
	}
}

// Called when local player character was spawned and possessed, so we can bind to data
void UMVVM_MyHUD::OnLocalPlayerReady(APlayerCharacter* PlayerCharacter)
{
	checkf(PlayerCharacter, TEXT("ERROR: [%i] %s:\n'PlayerCharacter' is null!"), __LINE__, *FString(__FUNCTION__));
	PlayerCharacter->OnPowerUpsChanged.AddUniqueDynamic(this, &ThisClass::OnPowerUpsChanged);

	AMyGameStateBase& MyGameState = AMyGameStateBase::Get();
	MyGameState.OnStartingTimerSecRemainChanged.AddUniqueDynamic(this, &ThisClass::OnStartingTimerSecRemainChanged);
	MyGameState.OnInGameTimerSecRemainChanged.AddUniqueDynamic(this, &ThisClass::OnInGameTimerSecRemainChanged);
}