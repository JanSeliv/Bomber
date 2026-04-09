// Copyright (c) Yevhenii Selivanov

#include "Widgets/NewMainMenuWidget.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "Data/NmmStateTag.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Components/BmrSkeletalMeshComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "Subsystems/BmrSoundsSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/Button.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuWidget)

// Called after the underlying slate widget is constructed
void UNewMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

// Called when the widget is removed from the viewport
void UNewMainMenuWidget::NativeDestruct()
{
	// Clear cached PlayButtonPressed so late-binding listeners receive fresh data on next menu load
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::PlayButtonPressed, this);

	Super::NativeDestruct();
}

// Is called when player pressed the button to start the game
void UNewMainMenuWidget::OnPlayButtonPressed()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPc' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	UBmrSoundsSubsystem::Get().PlayUIClickSFX();

	// In Idle+ states, validate spot and potentially run cinematic instead of broadcasting directly
	if (UNMMUtils::GetMainMenuState() != FNmmStateTag::BasicMenu)
	{
		const UNMMSpotComponent* MainMenuSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
		const FBmrCinematicRow& CinematicRow = MainMenuSpot ? MainMenuSpot->GetCinematicRow() : FBmrCinematicRow::Empty;
		if (!ensureMsgf(CinematicRow.IsValid(), TEXT("ASSERT: [%i] %hs:\n'CinematicRow' is not valid!"), __LINE__, __FUNCTION__)
		    || !MainMenuSpot->IsSpotAvailable())
		{
			return;
		}

		if (!MainMenuSpot->IsSpotSkinAvailable())
		{
			return;
		}

		if (!UNMMUtils::ShouldSkipCinematic(CinematicRow))
		{
			UNMMBaseSubsystem::Get().SetNewMainMenuState(FNmmStateTag::Cinematic);
			return;
		}
	}

	// Notify that user clicked Play button in BasicMenu or cinematic skipped
	FGameplayEventData EventData;
	EventData.EventTag = NmmGameplayTags::Event::PlayButtonPressed;
	EventData.Instigator = MyPC->GetPawn();
	UGlobalMessageSubsystem::BroadcastGlobalMessage(EventData);

	// This button might be pressed locally by user, send event to server if has no authority
	if (!MyPC->HasAuthority())
	{
		MyPC->ServerBroadcastMessage(EventData);
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

	// Switch the preview skin on the spot, skip if no skins available yet (DR rows not cached)
	UBmrSkeletalMeshComponent& MeshComp = MainMenuSpot->GetMeshChecked();
	const int32 SkinsNum = MeshComp.GetSkinTexturesNum();
	if (!ensureMsgf(SkinsNum > 0, TEXT("ASSERT: [%i] %hs:\nSkin button pressed, but no skins available for current spot!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const int32 NextSkinIndex = (MeshComp.GetAppliedSkinIndex() + 1) % SkinsNum;
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
