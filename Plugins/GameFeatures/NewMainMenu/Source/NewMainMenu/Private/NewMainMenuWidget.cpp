// Copyright (c) Yevhenii Selivanov

#include "NewMainMenuWidget.h"
//---
#include "Bomber.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/NewMainMenuSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NewMainMenuSubsystem.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/Button.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuWidget)

// Called after the underlying slate widget is constructed
void UNewMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		BindOnGameStateChanged(MyGameState);
	}
	else if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->OnGameStateCreated.AddUniqueDynamic(this, &ThisClass::BindOnGameStateChanged);
	}

	if (PlayButton)
	{
		PlayButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		PlayButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnPlayButtonPressed);
	}

	if (RightButton)
	{
		RightButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		RightButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnRightButtonPressed);
	}

	if (LeftButton)
	{
		LeftButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		LeftButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnLeftButtonPressed);
	}
}

// Called when the current game state was changed
void UNewMainMenuWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	// Show this widget in Menu game state
	SetVisibility(CurrentGameState == ECurrentGameState::Menu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called to start listening game state changes
void UNewMainMenuWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	// Handle current game state if initialized with delay
	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
}

// Is called when player pressed the button to start the game
void UNewMainMenuWidget::OnPlayButtonPressed()
{
	USoundsSubsystem::Get().PlayUIClickSFX();

	if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController())
	{
		MyPC->ServerSetGameState(ECurrentGameState::Cinematic);
	}
}

// Is called when player pressed the button to choose next player
void UNewMainMenuWidget::OnRightButtonPressed()
{
	static constexpr int32 NextPlayer = 1;
	SwitchCurrentPlayer(NextPlayer);
}

// Is called when player pressed the button to choose previous player
void UNewMainMenuWidget::OnLeftButtonPressed()
{
	static constexpr int32 PrevPlayer = -1;
	SwitchCurrentPlayer(PrevPlayer);
}

// Sets the preview mesh of a player depending on specified incrementer
void UNewMainMenuWidget::SwitchCurrentPlayer(int32 Incrementer)
{
	APlayerCharacter* LocalPlayerCharacter = GetOwningPlayerPawn<APlayerCharacter>();
	if (!ensureMsgf(LocalPlayerCharacter, TEXT("ASSERT: 'LocalPlayerState' is not valid"))
		|| !Incrementer)
	{
		return;
	}

	// Play the sound
	USoundsSubsystem::Get().PlayUIClickSFX();

	// Switch the Main Menu spot
	const UNewMainMenuSpotComponent* MainMenuSpot = UNewMainMenuSubsystem::Get().MoveMainMenuSpot(Incrementer);

	// Update the chosen player mesh on the level
	const FCustomPlayerMeshData& CustomPlayerMeshData = MainMenuSpot ? MainMenuSpot->GetMeshChecked().GetCustomPlayerMeshData() : FCustomPlayerMeshData::Empty;
	if (CustomPlayerMeshData.IsValid())
	{
		LocalPlayerCharacter->ServerSetCustomPlayerMeshData(CustomPlayerMeshData);
	}
}
