// Copyright (c) Yevhenii Selivanov

#include "Widgets/NewMainMenuWidget.h"
//---
#include "Bomber.h"
#include "NMMUtils.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMSubsystem.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuWidget)

// Called after the underlying slate widget is constructed
void UNewMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide this widget by default
	SetVisibility(ESlateVisibility::Collapsed);

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

	if (NextPlayerButton)
	{
		NextPlayerButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		NextPlayerButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnNextPlayerButtonPressed);
	}

	if (PrevPlayerButton)
	{
		PrevPlayerButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		PrevPlayerButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnPrevPlayerButtonPressed);
	}

	if (NextSkinButton)
	{
		NextSkinButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		NextSkinButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnNextSkinButtonPressed);
	}

	if (SettingsButton)
	{
		NextSkinButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		SettingsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSettingsButtonPressed);
	}

	if (QuitGameButton)
	{
		QuitGameButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		QuitGameButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnQuitGameButtonPressed);
	}
}

// Called when the current game state was changed
void UNewMainMenuWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	// Show this widget in Menu game state
	SetVisibility(CurrentGameState == ECurrentGameState::Menu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called to prepare the Main Menu widget for Menu game state
void UNewMainMenuWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	checkf(MyGameState, TEXT("ERROR: 'MyGameState' is null!"));

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);

	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		// Handle current game state if initialized with delay
		OnGameStateChanged(ECurrentGameState::Menu);
	}
}

// Is called when player pressed the button to start the game
void UNewMainMenuWidget::OnPlayButtonPressed()
{
	USoundsSubsystem::Get().PlayUIClickSFX();

	if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		// Start cinematic
		// If should skip, then start the game instead
		const UNMMSpotComponent* MainMenuSpot = UNMMSubsystem::Get().GetActiveMainMenuSpotComponent();
		const FNMMCinematicRow& CinematicRow = MainMenuSpot ? MainMenuSpot->GetCinematicRow() : FNMMCinematicRow::Empty;
		const ECGS NewState = !UNMMUtils::ShouldSkipCinematic(CinematicRow) ? ECGS::Cinematic : ECGS::GameStarting;
		MyPC->ServerSetGameState(NewState);
	}
}

// Is called when player pressed the button to choose next player
void UNewMainMenuWidget::OnNextPlayerButtonPressed()
{
	static constexpr int32 NextPlayer = 1;
	SwitchCurrentPlayer(NextPlayer);
}

// Is called when player pressed the button to choose previous player
void UNewMainMenuWidget::OnPrevPlayerButtonPressed()
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
	const UNMMSpotComponent* MainMenuSpot = UNMMSubsystem::Get().MoveMainMenuSpot(Incrementer);

	// Update the chosen player mesh on the level
	const FCustomPlayerMeshData& CustomPlayerMeshData = MainMenuSpot ? MainMenuSpot->GetMeshChecked().GetCustomPlayerMeshData() : FCustomPlayerMeshData::Empty;
	if (CustomPlayerMeshData.IsValid())
	{
		LocalPlayerCharacter->ServerSetCustomPlayerMeshData(CustomPlayerMeshData);
	}
}

// Sets the next skin in the Menu
void UNewMainMenuWidget::OnNextSkinButtonPressed()
{
	APlayerCharacter* LocalPlayerCharacter = GetOwningPlayerPawn<APlayerCharacter>();
	const UNMMSpotComponent* MainMenuSpot = UNMMSubsystem::Get().GetActiveMainMenuSpotComponent();
	if (!ensureMsgf(LocalPlayerCharacter, TEXT("ASSERT: 'LocalPlayerCharacter' is not valid"))
		|| !ensureMsgf(MainMenuSpot, TEXT("ASSERT: 'MainMenuSpot' is not valid")))
	{
		return;
	}

	UMySkeletalMeshComponent& MainMenuMeshComp = MainMenuSpot->GetMeshChecked();
	const FCustomPlayerMeshData& CustomPlayerMeshData = MainMenuMeshComp.GetCustomPlayerMeshData();
	if (!CustomPlayerMeshData.IsValid())
	{
		return;
	}

	USoundsSubsystem::Get().PlayUIClickSFX();

	// Switch the preview skin
	static constexpr int32 NextSkin = 1;
	const int32 NewSkinIndex = CustomPlayerMeshData.SkinIndex + NextSkin;
	MainMenuMeshComp.SetSkin(NewSkinIndex);

	// Update the player data
	LocalPlayerCharacter->ServerSetCustomPlayerMeshData(MainMenuMeshComp.GetCustomPlayerMeshData());
}

// Is called when player pressed the button to open the Settings
void UNewMainMenuWidget::OnSettingsButtonPressed()
{
	if (USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget())
	{
		SettingsWidget->OpenSettings();
	}
}

// Is called when player pressed the button to quit the game
void UNewMainMenuWidget::OnQuitGameButtonPressed()
{
	AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>();
	UKismetSystemLibrary::QuitGame(this, MyPC, EQuitPreference::Background, false);
}
