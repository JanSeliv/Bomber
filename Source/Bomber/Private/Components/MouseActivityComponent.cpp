// Copyright (c) Yevhenii Selivanov

#include "Components/MouseActivityComponent.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "GameFramework/PlayerController.h"
//---
#if WITH_EDITOR
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MouseActivityComponent)

// Sets default values for this component's properties
UMouseActivityComponent::UMouseActivityComponent()
{
	// Is ticking to calculate Delta Time
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

/*********************************************************************************************
 * Public functions
 ********************************************************************************************* */

// Returns Player Controller of this component
APlayerController* UMouseActivityComponent::GetPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

APlayerController& UMouseActivityComponent::GetPlayerControllerChecked() const
{
	APlayerController* MyPlayerController = GetPlayerController();
	checkf(MyPlayerController, TEXT("%s: 'MyPlayerController' is null"), *FString(__FUNCTION__));
	return *MyPlayerController;
}

// Returns true if the mouse cursor can be hidden
bool UMouseActivityComponent::CanHideMouse()
{
	switch (AMyGameStateBase::GetCurrentGameState())
	{
		case ECurrentGameState::GameStarting:
		case ECurrentGameState::InGame:
		case ECurrentGameState::Cinematic:
			return true;
		default:
			return false;
	}
}

// Called to to set mouse cursor visibility
void UMouseActivityComponent::SetMouseVisibility(bool bShouldShow)
{
	APlayerController& PC = GetPlayerControllerChecked();
	const bool bFailedToHide = !bShouldShow && !CanHideMouse();
	if (bFailedToHide
	    || !PC.IsLocalController())
	{
		return;
	}

	PC.SetShowMouseCursor(bShouldShow);
	PC.bEnableClickEvents = bShouldShow;
	PC.bEnableMouseOverEvents = bShouldShow;

	SetMouseFocusOnUI(bShouldShow);

	SetComponentTickEnabled(bShouldShow);
}

// If true, set the mouse focus on game and UI, otherwise only focusing on game inputs
void UMouseActivityComponent::SetMouseFocusOnUI(bool bFocusOnUI)
{
	APlayerController& PC = GetPlayerControllerChecked();

#if WITH_EDITOR // [IsEditorMultiplayer]
	if (FEditorUtilsLibrary::IsEditorMultiplayer())
	{
		const ULocalPlayer* LocalPlayer = PC.GetLocalPlayer();
		UGameViewportClient* GameViewport = LocalPlayer ? LocalPlayer->ViewportClient : nullptr;
		FViewport* Viewport = GameViewport ? GameViewport->Viewport : nullptr;
		if (!Viewport
		    || !GameViewport->IsFocused(Viewport))
		{
			// Do not change the focus for inactive viewports in editor-multiplayer
			// to avoid misleading focus on another game window
			return;
		}
	}
#endif // WITH_EDITOR [IsEditorMultiplayer]

	if (bFocusOnUI)
	{
		FInputModeGameAndUI GameAndUI;
		GameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC.SetInputMode(GameAndUI);
	}
	else
	{
		static const FInputModeGameOnly GameOnly{};
		PC.SetInputMode(GameOnly);
	}
}

/*********************************************************************************************
 * Protected functions
 ********************************************************************************************* */

// Called when the game starts
void UMouseActivityComponent::BeginPlay()
{
	Super::BeginPlay();

	SetMouseFocusOnUI(true);

	// Listen to handle input for each game state
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);

		// Handle current game state if initialized with delay
		if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
		{
			OnGameStateChanged(ECurrentGameState::Menu);
		}
	}
}

// Called every frame to calculate Delta Time
void UMouseActivityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Listen to toggle mouse visibility
void UMouseActivityComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
			SetMouseVisibility(true);
			break;
		case ECurrentGameState::Cinematic:
			SetMouseVisibility(false);
			break;
		case ECurrentGameState::GameStarting:
			SetMouseVisibility(false);
			break;
		case ECurrentGameState::InGame:
			SetMouseVisibility(false);
			break;
		case ECurrentGameState::EndGame:
			SetMouseVisibility(true);
			break;
		default:
			break;
	}
}
