// Copyright (c) Yevhenii Selivanov

#include "Widgets/NewMainMenuWidget.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "NMMUtils.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Components/BmrSkeletalMeshComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "Subsystems/BmrSoundsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuWidget)

// Called after the underlying slate widget is constructed
void UNewMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BIND_ON_MENU_STATE_CHANGED(this, ThisClass::OnNewMainMenuStateChanged);

	if (UNMMBaseSubsystem::Get().GetCurrentMenuState() == ENMMState::None)
	{
		// Hide this widget by default if is none state
		SetVisibility(ESlateVisibility::Collapsed);
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

void UNewMainMenuWidget::OnNewMainMenuStateChanged_Implementation(ENMMState NewState, ENMMState PreviousState)
{
	// Show this widget in Idle Menu state
	SetVisibility(NewState == ENMMState::Idle ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called when player pressed the button to start the game
void UNewMainMenuWidget::OnPlayButtonPressed()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPc' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const UNMMSpotComponent* MainMenuSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	const FNMMCinematicRow& CinematicRow = MainMenuSpot ? MainMenuSpot->GetCinematicRow() : FNMMCinematicRow::Empty;
	if (!ensureMsgf(CinematicRow.IsValid(), TEXT("ASSERT: [%i] %hs:\n'CinematicRow' is not valid!"), __LINE__, __FUNCTION__)
	    || !MainMenuSpot->IsSpotAvailable())
	{
		// The spot is locked
		return;
	}

	if (!MainMenuSpot->IsSpotSkinAvailable())
	{
		// the spot's skin unavailable
		return;
	}

	UBmrSoundsSubsystem::Get().PlayUIClickSFX();

	if (UNMMUtils::ShouldSkipCinematic(CinematicRow))
	{
		MyPC->SetGameStartingState();
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
	if (!Incrementer)
	{
		return;
	}

	// Play the sound
	UBmrSoundsSubsystem::Get().PlayUIClickSFX();

	// Switch the Main Menu spot
	UNMMSpotsSubsystem::Get().MoveMainMenuSpot(Incrementer);
}

// Sets the next skin in the Menu
void UNewMainMenuWidget::OnNextSkinButtonPressed()
{
	UNMMSpotComponent* MainMenuSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	if (!ensureMsgf(MainMenuSpot, TEXT("ASSERT: 'MainMenuSpot' is not valid"))
	    || !MainMenuSpot->IsSpotAvailable())
	{
		// The spot is locked
		return;
	}

	UBmrSoundsSubsystem::Get().PlayUIClickSFX();

	// Switch the preview skin on the spot
	UBmrSkeletalMeshComponent& MeshComp = MainMenuSpot->GetMeshChecked();
	const int32 NextSkinIndex = (MeshComp.GetAppliedSkinIndex() + 1) % MeshComp.GetSkinTexturesNum();
	MeshComp.ApplySkinByIndex(NextSkinIndex);

	// Update in-game player skin
	MainMenuSpot->ApplyMeshOnPlayer();
}

// Is called when player pressed the button to open the Settings
void UNewMainMenuWidget::OnSettingsButtonPressed()
{
	if (USettingsWidget* SettingsWidget = UBmrBlueprintFunctionLibrary::GetSettingsWidget())
	{
		SettingsWidget->OpenSettings();
	}
}

// Is called when player pressed the button to quit the game
void UNewMainMenuWidget::OnQuitGameButtonPressed()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	UKismetSystemLibrary::QuitGame(this, MyPC, EQuitPreference::Background, false);
}
