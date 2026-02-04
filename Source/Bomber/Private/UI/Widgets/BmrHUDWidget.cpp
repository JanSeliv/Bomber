// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/BmrHUDWidget.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Controllers/BmrPlayerController.h"
#include "GameFramework/BmrPlayerState.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrHUDWidget)

//  Called after the underlying slate widget is constructed
void UBmrHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPlayerStateReady);

	if (RestartButton)
	{
		RestartButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		RestartButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnRestartButtonPressed);
	}

	if (MenuButton)
	{
		MenuButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		MenuButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnMenuButtonPressed);
	}

	if (SettingsButton)
	{
		SettingsButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		SettingsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSettingsButtonPressed);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the local player state is initialized and its assigned character is ready
void UBmrHUDWidget::OnLocalPlayerStateReady_Implementation(const FGameplayEventData& Payload)
{
	// Listen the ending the current game to play the End-Game sound on
	const APawn* Pawn = Cast<APawn>(Payload.Instigator.Get());
	ABmrPlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<ABmrPlayerState>() : nullptr;
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Is called on end-game result change
void UBmrHUDWidget::OnEndGameStateChanged_Implementation(EBmrEndGameState EndGameState)
{
	if (EndGameState != EBmrEndGameState::None)
	{
		PlayAnimation(ResultAnimation);
	}
}

// Is called when player pressed the button to restart the match
void UBmrHUDWidget::OnRestartButtonPressed_Implementation()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!MyPC)
	{
		return;
	}

	// This button might be pressed locally by user, send event to server if has no authority
	FGameplayEventData EventData;
	EventData.EventTag = BmrGameplayTags::Event::HUD_RestartButtonPressed;
	EventData.Instigator = MyPC->GetPawn();
	UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);
	if (!MyPC->HasAuthority())
	{
		MyPC->ServerSendGameplayEvent(EventData);
	}

	MyPC->SetGameStartingState();
}

// Is called when player pressed the button to return to the Main Menu
void UBmrHUDWidget::OnMenuButtonPressed_Implementation()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!MyPC)
	{
		return;
	}

	// This button might be pressed locally by user, send event to server if has no authority
	FGameplayEventData EventData;
	EventData.EventTag = BmrGameplayTags::Event::HUD_MenuButtonPressed;
	EventData.Instigator = MyPC->GetPawn();
	UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);
	if (!MyPC->HasAuthority())
	{
		MyPC->ServerSendGameplayEvent(EventData);
	}

	MyPC->SetMenuState();
}

// Is called when player pressed the button to open the Settings
void UBmrHUDWidget::OnSettingsButtonPressed_Implementation()
{
	if (USettingsWidget* SettingsWidget = UBmrBlueprintFunctionLibrary::GetSettingsWidget())
	{
		SettingsWidget->OpenSettings();
	}
}