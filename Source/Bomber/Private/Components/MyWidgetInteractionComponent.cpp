// Copyright (c) Yevhenii Selivanov

#include "Components/MyWidgetInteractionComponent.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "Controllers/MyPlayerController.h"
#include "Globals/SingletonLibrary.h"

// Sets default values for this component's properties
UMyWidgetInteractionComponent::UMyWidgetInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TraceChannel = ECC_UI;
	InteractionDistance = 50000.f;
	InteractionSource = EWidgetInteractionSource::Mouse;
}

// Sets most suitable Virtual User index by current player index
void UMyWidgetInteractionComponent::UpdatePlayerIndex()
{
	int32 IndexToSet = 0;

#if WITH_EDITOR // [IsEditorMultiplayer]
	if (USingletonLibrary::IsEditorMultiplayer())
	{
		// Make widget interaction works in editor multiplayer,
		// so interaction will not conflict with every local player
		IndexToSet = USingletonLibrary::GetEditorPlayerIndex();
	}
#endif // [IsEditorMultiplayer]

	VirtualUserIndex = IndexToSet;
	PointerIndex = IndexToSet;
}

// Called when the game starts
void UMyWidgetInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	UpdatePlayerIndex();

	EnableInput();
}

// Listen game states to manage the enabling and disabling this component
void UMyWidgetInteractionComponent::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		{
			SetActive(true);
			break;
		}
		case ECurrentGameState::GameStarting:
		{
			SetActive(false);
			break;
		}
		default:
			break;
	}
}

// Pushes the owner actor to the stack of input to be able to send input key events
void UMyWidgetInteractionComponent::EnableInput()
{
	if (AActor* Owner = GetOwner())
	{
		AMyPlayerController* PC = USingletonLibrary::GetLocalPlayerController();
		Owner->EnableInput(PC);
	}
}

// Sends the press key event to slate, is overriden to ignore if component is not active
bool UMyWidgetInteractionComponent::PressKey(FKey Key, bool bRepeat)
{
	return IsActive() ? Super::PressKey(Key, bRepeat) : false;
}

// Sends the release key event to slate, is overriden to ignore if component is not active
bool UMyWidgetInteractionComponent::ReleaseKey(FKey Key)
{
	return IsActive() ? Super::ReleaseKey(Key) : false;
}
