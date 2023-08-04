// Copyright (c) Yevhenii Selivanov

#include "Components/MenuWidgetInteractionComponent.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "UI/MyHUD.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MenuWidgetInteractionComponent)

// Sets default values for this component's properties
UMenuWidgetInteractionComponent::UMenuWidgetInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TraceChannel = ECC_UI;
	InteractionDistance = 50000.f;
	InteractionSource = EWidgetInteractionSource::Mouse;
}

// Sends the press key event to slate, is overriden to ignore if component is not active
bool UMenuWidgetInteractionComponent::PressKey(FKey Key, bool bRepeat)
{
	return IsActive() ? Super::PressKey(Key, bRepeat) : false;
}

// Sends the release key event to slate, is overriden to ignore if component is not active
bool UMenuWidgetInteractionComponent::ReleaseKey(FKey Key)
{
	return IsActive() ? Super::ReleaseKey(Key) : false;
}

// Sets most suitable Virtual User index by current player index
void UMenuWidgetInteractionComponent::UpdatePlayerIndex()
{
	int32 IndexToSet = 0;

#if WITH_EDITOR // [IsEditorMultiplayer]
	if (FEditorUtilsLibrary::IsEditorMultiplayer())
	{
		// Make widget interaction works in editor multiplayer,
		// so interaction will not conflict with every local player
		IndexToSet = FEditorUtilsLibrary::GetEditorPlayerIndex();
	}
#endif // [IsEditorMultiplayer]

	VirtualUserIndex = IndexToSet;
	PointerIndex = IndexToSet;
}

// Called when the game starts
void UMenuWidgetInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	UpdatePlayerIndex();

	EnableInput();

	BindOnToggledSettings();
}

// Presses a key as if the mouse/pointer were the source of it
void UMenuWidgetInteractionComponent::PressPointerKey(FKey Key)
{
	// Do not call super, override behavior instead

	if (!VirtualUser.IsValid()
	    || !CanSendInput()
	    || PressedKeys.Contains(Key))
	{
		return;
	}

	PressedKeys.Emplace(Key);

	FPointerEvent PointerEvent;
	const uint32 InUserIndex = static_cast<uint32>(VirtualUser->GetUserIndex());
	const uint32 InPointerIndex = static_cast<uint32>(PointerIndex);
	if (Key.IsTouch())
	{
		constexpr float InForce = 1.f;
		constexpr bool bPressLeftMouseButton = false;
		PointerEvent = {
			InUserIndex,
			InPointerIndex,
			LocalHitLocation,
			LastLocalHitLocation,
			InForce,
			bPressLeftMouseButton};
	}
	else
	{
		constexpr float InWheelDelta = 0.f;
		PointerEvent = {
			InUserIndex,
			InPointerIndex,
			LocalHitLocation,
			LastLocalHitLocation,
			PressedKeys,
			Key,
			InWheelDelta,
			ModifierKeys};
	}

	FSlateApplication::Get().RoutePointerDownEvent(LastWidgetPath.ToWidgetPath(), PointerEvent);
}

// Listen game states to manage the enabling and disabling this component
void UMenuWidgetInteractionComponent::OnGameStateChanged(ECurrentGameState CurrentGameState)
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
void UMenuWidgetInteractionComponent::EnableInput()
{
	if (AActor* Owner = GetOwner())
	{
		AMyPlayerController* PC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
		Owner->EnableInput(PC);
	}
}

// Binds to toggle Settings to be able enable or disable this component
void UMenuWidgetInteractionComponent::BindOnToggledSettings()
{
	if (USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget())
	{
		SettingsWidget->OnToggledSettings.AddUniqueDynamic(this, &ThisClass::OnToggledSettings);
		return;
	}

	// Settings widget is not valid yet, so wait until it becomes initialized
	AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD();
	if (MyHUD && !MyHUD->AreWidgetInitialized())
	{
		MyHUD->OnWidgetsInitialized.AddUniqueDynamic(this, &ThisClass::OnWidgetsInitialized);
	}
}

// Is called when all widgets are initialized to bind on settings toggle
void UMenuWidgetInteractionComponent::OnWidgetsInitialized()
{
	AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD();
	if (MyHUD
	    && MyHUD->OnWidgetsInitialized.IsAlreadyBound(this, &ThisClass::OnWidgetsInitialized))
	{
		MyHUD->OnWidgetsInitialized.RemoveDynamic(this, &ThisClass::OnWidgetsInitialized);
	}

	BindOnToggledSettings();
}

// Disables this component while setting are opened and vice versa.
void UMenuWidgetInteractionComponent::OnToggledSettings(bool bIsVisible)
{
	SetActive(!bIsVisible);
}
