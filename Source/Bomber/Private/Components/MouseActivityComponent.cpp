﻿// Copyright (c) Yevhenii Selivanov

#include "Components/MouseActivityComponent.h"
//---
#include "DataAssets/PlayerInputDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/GlobalEventsSubsystem.h"
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

// Returns current mouse visibility settings
const FMouseVisibilitySettings& UMouseActivityComponent::GetCurrentVisibilitySettings() const
{
	return CurrentVisibilitySettingsInternal;
}

// Applies the new mouse visibility settings
void UMouseActivityComponent::SetMouseVisibilitySettings(const FMouseVisibilitySettings& NewSettings)
{
	if (NewSettings.IsValid())
	{
		CurrentVisibilitySettingsInternal = NewSettings;
		SetMouseVisibility(NewSettings.bIsVisible);
	}
}

// Called to to set mouse cursor visibility
void UMouseActivityComponent::SetMouseVisibility(bool bShouldShow)
{
	APlayerController& PC = GetPlayerControllerChecked();
	if (!PC.IsLocalController())
	{
		return;
	}

	PC.SetShowMouseCursor(bShouldShow);
	PC.bEnableClickEvents = bShouldShow;
	PC.bEnableMouseOverEvents = bShouldShow;

	SetMouseFocusOnUI(bShouldShow);

	SetComponentTickEnabled(bShouldShow);

	CurrentlyInactiveSecInternal = 0.f;

	if (OnMouseVisibilityChanged.IsBound())
	{
		OnMouseVisibilityChanged.Broadcast(bShouldShow);
	}
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
	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Called every frame to calculate Delta Time
void UMouseActivityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FMouseVisibilitySettings& VisibilitySettings = GetCurrentVisibilitySettings();
	if (VisibilitySettings.IsInactivityEnabled())
	{
		CurrentlyInactiveSecInternal += DeltaTime;

		if (CurrentlyInactiveSecInternal >= VisibilitySettings.SecToAutoHide)
		{
			SetMouseVisibility(false);
		}
	}
}

// Is called from input mouse event to reset inactivity time
void UMouseActivityComponent::OnMouseMove_Implementation()
{
	CurrentlyInactiveSecInternal = 0.f;

	if (!GetPlayerControllerChecked().ShouldShowMouseCursor()
	    && GetCurrentVisibilitySettings().bIsVisible)
	{
		// Show inactive mouse
		SetMouseVisibility(true);
	}
}

// Listen to toggle mouse visibility
void UMouseActivityComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	const FMouseVisibilitySettings& NewSettings = UPlayerInputDataAsset::Get().GetMouseVisibilitySettings(CurrentGameState);
	SetMouseVisibilitySettings(NewSettings);
}
