// Copyright (c) Yevhenii Selivanov

#include "Widgets/NewMainMenuWidget.h"
//---
#include "Bomber.h"
#include "NMMUtils.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"
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

	// Listen Main Menu states
	UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();
	BaseSubsystem.OnMainMenuStateChanged.AddUniqueDynamic(this, &ThisClass::OnNewMainMenuStateChanged);
	if (BaseSubsystem.GetCurrentMenuState() != ENMMState::None)
	{
		// State is already set, apply it
		OnNewMainMenuStateChanged(BaseSubsystem.GetCurrentMenuState());
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

void UNewMainMenuWidget::OnNewMainMenuStateChanged_Implementation(ENMMState NewState)
{
	// Show this widget in Idle Menu state
	SetVisibility(NewState == ENMMState::Idle ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called when player pressed the button to start the game
void UNewMainMenuWidget::OnPlayButtonPressed()
{
	AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>();
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %s:\n'MyPc' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const UNMMSpotComponent* MainMenuSpot = UNMMSpotsSubsystem::Get().GetActiveMainMenuSpotComponent();
	const FNMMCinematicRow& CinematicRow = MainMenuSpot ? MainMenuSpot->GetCinematicRow() : FNMMCinematicRow::Empty;
	if (!ensureMsgf(CinematicRow.IsValid(), TEXT("ASSERT: [%i] %s:\n'CinematicRow' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| !MainMenuSpot->GetMeshChecked().IsVisible())
	{
		// The spot is locked
		return;
	}

	USoundsSubsystem::Get().PlayUIClickSFX();

	if (UNMMUtils::ShouldSkipCinematic(CinematicRow))
	{
		MyPC->ServerSetGameState(ECurrentGameState::GameStarting);
	}
	else
	{
		UNMMBaseSubsystem::Get().SetNewMainMenuState(ENMMState::Cinematic);
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
	const UNMMSpotComponent* MainMenuSpot = UNMMSpotsSubsystem::Get().MoveMainMenuSpot(Incrementer);

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
	const UNMMSpotComponent* MainMenuSpot = UNMMSpotsSubsystem::Get().GetActiveMainMenuSpotComponent();
	if (!ensureMsgf(LocalPlayerCharacter, TEXT("ASSERT: 'LocalPlayerCharacter' is not valid"))
		|| !ensureMsgf(MainMenuSpot, TEXT("ASSERT: 'MainMenuSpot' is not valid")))
	{
		return;
	}

	UMySkeletalMeshComponent& MainMenuMeshComp = MainMenuSpot->GetMeshChecked();
	const FCustomPlayerMeshData& CustomPlayerMeshData = MainMenuMeshComp.GetCustomPlayerMeshData();
	if (!ensureMsgf(CustomPlayerMeshData.IsValid(), TEXT("ASSERT: 'CustomPlayerMeshData' is not valid"))
		|| !MainMenuMeshComp.IsVisible())
	{
		// The spot is locked
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
