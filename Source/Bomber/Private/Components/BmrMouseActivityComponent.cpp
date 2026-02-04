// Copyright (c) Yevhenii Selivanov

#include "Components/BmrMouseActivityComponent.h"

// Bomber
#include "DataAssets/BmrPlayerInputDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMouseActivityComponent)

// Sets default values for this component's properties
UBmrMouseActivityComponent::UBmrMouseActivityComponent()
{
	// Is ticking to calculate Delta Time
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

/*********************************************************************************************
 * Public functions
 ********************************************************************************************* */

// Returns Player Controller of this component
APlayerController* UBmrMouseActivityComponent::GetPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

APlayerController& UBmrMouseActivityComponent::GetPlayerControllerChecked() const
{
	APlayerController* MyPlayerController = GetPlayerController();
	checkf(MyPlayerController, TEXT("%s: 'MyPlayerController' is null"), *FString(__FUNCTION__));
	return *MyPlayerController;
}

// Returns current mouse visibility settings
const FBmrMouseVisibilitySettings& UBmrMouseActivityComponent::GetCurrentVisibilitySettings() const
{
	return CurrentVisibilitySettings;
}

// Applies the new mouse visibility settings
void UBmrMouseActivityComponent::SetMouseVisibilitySettingsEnabled(bool bEnable, EBmrCurrentGameState GameState)
{
	if (bEnable)
	{
		const FBmrMouseVisibilitySettings& NewSettings = UBmrPlayerInputDataAsset::Get().GetMouseVisibilitySettings(GameState);
		EnableMouseVisibilitySettings(NewSettings);
	}
	else if (CurrentVisibilitySettings.GameState == GameState)
	{
		DisableMouseVisibilitySettings();
	}
}

// Applies the new mouse visibility settings by custom game state
void UBmrMouseActivityComponent::SetMouseVisibilitySettingsEnabledCustom(bool bEnable, FName CustomGameState)
{
	if (bEnable)
	{
		const FBmrMouseVisibilitySettings& NewSettings = UBmrPlayerInputDataAsset::Get().GetMouseVisibilitySettingsCustom(CustomGameState);
		EnableMouseVisibilitySettings(NewSettings);
	}
	else if (CurrentVisibilitySettings.CustomGameState == CustomGameState)
	{
		DisableMouseVisibilitySettings();
	}
}

/*********************************************************************************************
 * Protected functions
 ********************************************************************************************* */

// Applies the new mouse visibility settings
void UBmrMouseActivityComponent::EnableMouseVisibilitySettings(const FBmrMouseVisibilitySettings& NewSettings)
{
	if (ensureMsgf(NewSettings.IsValid(), TEXT("ASSERT: [%i] %hs:\n'NewSettings' is not valid!"), __LINE__, __FUNCTION__))
	{
		PreviousVisibilitySettings = CurrentVisibilitySettings;
		CurrentVisibilitySettings = NewSettings;
		SetMouseVisibility(NewSettings.bIsVisible);
	}
}

// Restores previous mouse visibility settings
void UBmrMouseActivityComponent::DisableMouseVisibilitySettings()
{
	if (PreviousVisibilitySettings.IsValid())
	{
		CurrentVisibilitySettings = PreviousVisibilitySettings;
		PreviousVisibilitySettings = FBmrMouseVisibilitySettings::Invalid;
		SetMouseVisibility(CurrentVisibilitySettings.bIsVisible);
	}
}

// Called to set mouse cursor visibility
void UBmrMouseActivityComponent::SetMouseVisibility(bool bShouldShow)
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

	CurrentlyInactiveSec = 0.f;

	if (OnMouseVisibilityChanged.IsBound())
	{
		OnMouseVisibilityChanged.Broadcast(bShouldShow);
	}
}

// If true, set the mouse focus on game and UI, otherwise only focusing on game inputs
void UBmrMouseActivityComponent::SetMouseFocusOnUI(bool bFocusOnUI)
{
	APlayerController& PC = GetPlayerControllerChecked();

	if (UUtilsLibrary::IsPIE()
	    && UMultiplayerUtilsLibrary::IsMultiplayerGame()
	    && !UUtilsLibrary::IsViewportFocused())
	{
		// In editor multiplayer, do not change focus for non-focused instances, since it steals focus from the current window
		return;
	}

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
void UBmrMouseActivityComponent::TickHandleInactivity(float DeltaTime)
{
	CurrentlyInactiveSec += DeltaTime;

	// Check if mouse position has changed
	if (const APlayerController* PlayerController = GetPlayerController())
	{
		FVector2D OutMousePosition;
		PlayerController->GetMousePosition(OutMousePosition.X, OutMousePosition.Y);
		if (OutMousePosition != LastMousePosition)
		{
			OnMouseMove();
			LastMousePosition = OutMousePosition;
		}
	}

	if (CurrentlyInactiveSec >= CurrentVisibilitySettings.SecToAutoHide)
	{
		SetMouseVisibility(false);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the game starts
void UBmrMouseActivityComponent::BeginPlay()
{
	Super::BeginPlay();

	SetMouseFocusOnUI(true);

	// Listen to handle input for each game state
	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Called every frame to calculate Delta Time
void UBmrMouseActivityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentVisibilitySettings.IsInactivityEnabled())
	{
		TickHandleInactivity(DeltaTime);
	}
}

// Is called from input mouse event to reset inactivity time
void UBmrMouseActivityComponent::OnMouseMove_Implementation()
{
	CurrentlyInactiveSec = 0.f;

	if (!GetPlayerControllerChecked().ShouldShowMouseCursor()
	    && GetCurrentVisibilitySettings().bIsVisible)
	{
		// Show inactive mouse
		SetMouseVisibility(true);
	}
}

// Listen to toggle mouse visibility
void UBmrMouseActivityComponent::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const EBmrCurrentGameState CurrentGameState = ABmrGameState::GetCurrentGameState();
	SetMouseVisibilitySettingsEnabled(true, CurrentGameState);
}