// Copyright (c) Yevhenii Selivanov.

#include "UI/MainMenuWidget.h"
//---
#include "GeneratedMap.h"
#include "Subsystems/SoundsSubsystem.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "LevelActors/PlayerCharacter.h"
#include "UI/Carousel.h"

// Initializes the main menu widget
void UMainMenuWidget::InitMainMenuWidget(ACarousel* InMainMenuActor)
{
	if (!ensureMsgf(InMainMenuActor, TEXT("ASSERT: 'InMainMenuActor' is not valid")))
	{
		return;
	}

	MainMenuActorInternal = InMainMenuActor;

	if (OnMainMenuReady.IsBound())
	{
		OnMainMenuReady.Broadcast();
	}
}

// Sets the next player in the Menu
void UMainMenuWidget::ChooseRight()
{
	static constexpr int32 NextPlayer = 1;
	SwitchCurrentPlayer(NextPlayer);
}

// Sets the previous player in the Menu
void UMainMenuWidget::ChooseLeft()
{
	static constexpr int32 PrevPlayer = -1;
	SwitchCurrentPlayer(PrevPlayer);
}

// Sets the previous or next player in the Menu
void UMainMenuWidget::ChooseRightLeft(const FInputActionValue& ActionValue)
{
	const float ScaleValue = ActionValue.GetMagnitude();
	if (!ScaleValue)
	{
		return;
	}

	if (ScaleValue < 0.f)
	{
		ChooseLeft();
	}
	else
	{
		ChooseRight();
	}
}

// Sets the next level in the Menu
void UMainMenuWidget::ChooseForward()
{
	static constexpr int32 NextLevel = 1;
	SwitchCurrentLevel(NextLevel);
}

// Sets the previous level in the Menu
void UMainMenuWidget::ChooseBack()
{
	static constexpr int32 PrevLevel = -1;
	SwitchCurrentLevel(PrevLevel);
}

// Sets the previous or next level in the Menu
void UMainMenuWidget::ChooseBackForward(const FInputActionValue& ActionValue)
{
	const float ScaleValue = ActionValue.GetMagnitude();
	if (!ScaleValue)
	{
		return;
	}

	if (ScaleValue < 0.f)
	{
		ChooseBack();
	}
	else
	{
		ChooseForward();
	}
}

// Sets the next skin in the Menu
void UMainMenuWidget::NextSkin()
{
	APlayerCharacter* LocalPlayerCharacter = GetOwningPlayerPawn<APlayerCharacter>();
	if (!ensureMsgf(LocalPlayerCharacter, TEXT("ASSERT: 'LocalPlayerState' is not valid")))
	{
		return;
	}

	UMySkeletalMeshComponent* MainMenuMeshComp = MainMenuActorInternal ? MainMenuActorInternal->GetCurrentMeshComponent<UMySkeletalMeshComponent>() : nullptr;
	const FCustomPlayerMeshData& CustomPlayerMeshData = MainMenuMeshComp ? MainMenuMeshComp->GetCustomPlayerMeshData() : FCustomPlayerMeshData::Empty;
	if (!CustomPlayerMeshData.IsValid())
	{
		return;
	}

	USoundsSubsystem::Get().PlayUIClickSFX();

	// Switch the preview skin
	const int32 NewSkinIndex = CustomPlayerMeshData.SkinIndex + 1;
	MainMenuMeshComp->SetSkin(NewSkinIndex);

	// Update the player data
	LocalPlayerCharacter->ServerSetCustomPlayerMeshData(CustomPlayerMeshData);
}

// Set the chosen on UI the level type
void UMainMenuWidget::ChooseNewLevel(ELevelType LevelType)
{
	USoundsSubsystem::Get().PlayUIClickSFX();

	AGeneratedMap::Get().SetLevelType(LevelType);
}

// Is executed when player pressed the button of starting the game
void UMainMenuWidget::StartGame()
{
	USoundsSubsystem::Get().PlayUIClickSFX();

	if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController())
	{
		MyPC->SetGameStartingState();
	}
}

// Is executed when player decided to close the game
void UMainMenuWidget::QuitGame()
{
	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	UKismetSystemLibrary::QuitGame(this, MyPC, EQuitPreference::Background, false);
}

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UMainMenuWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

// Updates appearance dynamically in the editor
void UMainMenuWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

// Sets the level depending on specified incrementer
void UMainMenuWidget::SwitchCurrentLevel(int32 Incrementer)
{
	const ELevelType CurrentLevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	if (CurrentLevelType == ELT::None
	    || !Incrementer)
	{
		return;
	}

	int32 NewLevelFlag = 0;
	const int32 CurrentLevelFlag = TO_FLAG(CurrentLevelType);

	if (Incrementer < 0)
	{
		Incrementer = FMath::Abs(Incrementer);
		NewLevelFlag = CurrentLevelFlag >> Incrementer;
		if (NewLevelFlag < ELT_FIRST_FLAG)
		{
			NewLevelFlag = ELT_LAST_FLAG;
		}
	}
	else
	{
		NewLevelFlag = CurrentLevelFlag << Incrementer;
		if (NewLevelFlag > ELT_LAST_FLAG)
		{
			NewLevelFlag = ELT_FIRST_FLAG;
		}
	}

	const ELevelType NewLevelType = TO_ENUM(ELevelType, NewLevelFlag);
	ChooseNewLevel(NewLevelType);
}

// Sets the preview mesh of a player depending on specified incrementer
void UMainMenuWidget::SwitchCurrentPlayer(int32 Incrementer)
{
	APlayerCharacter* LocalPlayerCharacter = GetOwningPlayerPawn<APlayerCharacter>();
	if (!ensureMsgf(LocalPlayerCharacter, TEXT("ASSERT: 'LocalPlayerState' is not valid"))
	    || !ensureMsgf(MainMenuActorInternal, TEXT("ASSERT: 'MainMenuActorInternal' is not valid"))
	    || !Incrementer)
	{
		return;
	}

	// Play the sound
	USoundsSubsystem::Get().PlayUIClickSFX();

	const bool bRotated = MainMenuActorInternal->RotateFloorBP(Incrementer);
	if (!bRotated)
	{
		return;
	}

	const UMySkeletalMeshComponent* MainMenuMeshComp = MainMenuActorInternal ? MainMenuActorInternal->GetCurrentMeshComponent<UMySkeletalMeshComponent>() : nullptr;
	const FCustomPlayerMeshData& CustomPlayerMeshData = MainMenuMeshComp ? MainMenuMeshComp->GetCustomPlayerMeshData() : FCustomPlayerMeshData::Empty;
	if (!CustomPlayerMeshData.IsValid())
	{
		return;
	}

	// Update player data
	LocalPlayerCharacter->ServerSetCustomPlayerMeshData(CustomPlayerMeshData);
}

// Called when the current game state was changed
void UMainMenuWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		{
			SetVisibility(ESlateVisibility::Visible);
			break;
		}
		case ECurrentGameState::GameStarting:
		{
			break;
		}
		case ECurrentGameState::EndGame:
		{
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
		default:
			break;
	}
}
