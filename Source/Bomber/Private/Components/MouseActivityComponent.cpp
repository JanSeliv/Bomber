// Copyright (c) Yevhenii Selivanov

#include "Components/MouseActivityComponent.h"
//---
#include "DataAssets/PlayerInputDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "GameFramework/PlayerController.h"
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
void UMouseActivityComponent::SetMouseVisibilitySettingsEnabled(bool bEnable, ECurrentGameState GameState)
{
	if (bEnable)
	{
		const FMouseVisibilitySettings& NewSettings = UPlayerInputDataAsset::Get().GetMouseVisibilitySettings(GameState);
		EnableMouseVisibilitySettings(NewSettings);
	}
	else if (CurrentVisibilitySettingsInternal.GameState == GameState)
	{
		DisableMouseVisibilitySettings();
	}
}

// Applies the new mouse visibility settings by custom game state
void UMouseActivityComponent::SetMouseVisibilitySettingsEnabledCustom(bool bEnable, FName CustomGameState)
{
	if (bEnable)
	{
		const FMouseVisibilitySettings& NewSettings = UPlayerInputDataAsset::Get().GetMouseVisibilitySettingsCustom(CustomGameState);
		EnableMouseVisibilitySettings(NewSettings);
	}
	else if (CurrentVisibilitySettingsInternal.CustomGameState == CustomGameState)
	{
		DisableMouseVisibilitySettings();
	}
}

/*********************************************************************************************
 * Protected functions
 ********************************************************************************************* */

// Applies the new mouse visibility settings
void UMouseActivityComponent::EnableMouseVisibilitySettings(const FMouseVisibilitySettings& NewSettings)
{
	if (ensureMsgf(NewSettings.IsValid(), TEXT("ASSERT: [%i] %hs:\n'NewSettings' is not valid!"), __LINE__, __FUNCTION__))
	{
		PreviousVisibilitySettingsInternal = CurrentVisibilitySettingsInternal;
		CurrentVisibilitySettingsInternal = NewSettings;
		SetMouseVisibility(NewSettings.bIsVisible);
	}
}

// Restores previous mouse visibility settings
void UMouseActivityComponent::DisableMouseVisibilitySettings()
{
	if (PreviousVisibilitySettingsInternal.IsValid())
	{
		CurrentVisibilitySettingsInternal = PreviousVisibilitySettingsInternal;
		PreviousVisibilitySettingsInternal = FMouseVisibilitySettings::Invalid;
		SetMouseVisibility(CurrentVisibilitySettingsInternal.bIsVisible);
	}
}

// Called to set mouse cursor visibility
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

// Is called in tick to detect mouse movement and handle inactivity
void UMouseActivityComponent::TickHandleInactivity(float DeltaTime)
{
	CurrentlyInactiveSecInternal += DeltaTime;

	// Check if mouse position has changed
	if (const APlayerController* PlayerController = GetPlayerController())
	{
		FVector2D OutMousePosition;
		PlayerController->GetMousePosition(OutMousePosition.X, OutMousePosition.Y);
		if (OutMousePosition != LastMousePositionInternal)
		{
			OnMouseMove();
			LastMousePositionInternal = OutMousePosition;
		}
	}

	if (CurrentlyInactiveSecInternal >= CurrentVisibilitySettingsInternal.SecToAutoHide)
	{
		SetMouseVisibility(false);
	}
}

/*********************************************************************************************
 * Overrides
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

	if (CurrentVisibilitySettingsInternal.IsInactivityEnabled())
	{
		TickHandleInactivity(DeltaTime);
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
	SetMouseVisibilitySettingsEnabled(true, CurrentGameState);
}