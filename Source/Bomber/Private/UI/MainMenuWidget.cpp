// Copyright 2021 Yevhenii Selivanov.

#include "UI/MainMenuWidget.h"
//---
#include "GeneratedMap.h"
#include "Components/MySkeletalMeshComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/SingletonLibrary.h"
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
void UMainMenuWidget::ChooseRight_Implementation()
{
	// BP implementation
	// ...
}

// Sets the previous player in the Menu
void UMainMenuWidget::ChooseLeft_Implementation()
{
	// BP implementation
	// ...
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

// Sets the next skin in the Menu
void UMainMenuWidget::NextSkin()
{
	UMySkeletalMeshComponent* MySkeletalMeshComponent = MainMenuActorInternal ? MainMenuActorInternal->GetCurrentMeshComponent<UMySkeletalMeshComponent>() : nullptr;
	const FCustomPlayerMeshData& CustomPlayerMeshData = MySkeletalMeshComponent ? MySkeletalMeshComponent->GetCustomPlayerMeshData() : FCustomPlayerMeshData::Empty;
	if (!CustomPlayerMeshData.IsValid())
	{
		return;
	}

	const int32 NewSkinIndex = CustomPlayerMeshData.SkinIndex + 1;
	MySkeletalMeshComponent->SetSkin(NewSkinIndex);

	if (AMyPlayerState* MyPlayerState = USingletonLibrary::GetCurrentPlayerState())
	{
		MyPlayerState->SetCustomPlayerMeshData(CustomPlayerMeshData);
	}
}

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UMainMenuWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
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
	const ELevelType CurrentLevelType = USingletonLibrary::GetLevelType();
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
	AGeneratedMap::Get().SetLevelType(NewLevelType);
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
