// Copyright (c) Yevhenii Selivanov.

#include "Subsystems/WidgetsSubsystem.h"
//---
#include "Controllers/MyPlayerController.h"
#include "DataAssets/UIDataAsset.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
#include "UI/SettingsWidget.h"
#include "UI/Widgets/HUDWidget.h"
#include "UI/Widgets/PlayerName3DWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/Viewport.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(WidgetsSubsystem)

// Returns the pointer the UI Subsystem
UWidgetsSubsystem* UWidgetsSubsystem::GetWidgetsSubsystem(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(OptionalWorldContext);
	if (!LocalPlayer)
	{
		const AMyPlayerController* PC = UMyBlueprintFunctionLibrary::GetLocalPlayerController(OptionalWorldContext);
		LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	}
	return LocalPlayer ? LocalPlayer->GetSubsystem<UWidgetsSubsystem>() : nullptr;
}

// Returns the UI subsystem checked: it will crash if player controller is not initialized yet
UWidgetsSubsystem& UWidgetsSubsystem::Get(const UObject* OptionalWorldContext)
{
	UWidgetsSubsystem* WidgetsSubsystem = GetWidgetsSubsystem(OptionalWorldContext);
	checkf(WidgetsSubsystem, TEXT("%s: 'WidgetsSubsystem' is null, likely controller is not initialized yet!"), *FString(__FUNCTION__));
	return *WidgetsSubsystem;
}

/*********************************************************************************************
 * Widgets Management
 ********************************************************************************************* */

// Adds given widget to the list of manageable widgets, so its visibility can be changed globally
void UWidgetsSubsystem::RegisterManageableWidget(UUserWidget* Widget)
{
	if (ensureMsgf(Widget, TEXT("ASSERT: [%i] %hs:\n'Widget' is null, can't register!"), __LINE__, __FUNCTION__))
	{
		AllManageableWidgetsInternal.Add(Widget);
	}
}

// Create specified widget and add it to Manageable widgets list, so its visibility can be changed globally
UUserWidget* UWidgetsSubsystem::CreateManageableWidget(TSubclassOf<UUserWidget> WidgetClass, bool bAddToViewport, int32 ZOrder, const UObject* OptionalWorldContext)
{
	UUserWidget* Widget = FWidgetUtilsLibrary::CreateWidgetByClass(WidgetClass, bAddToViewport, ZOrder, OptionalWorldContext);
	RegisterManageableWidget(Widget);
	return Widget;
}

// Removes given widget from the list and destroys it
void UWidgetsSubsystem::DestroyManageableWidget(UUserWidget* Widget)
{
	if (!Widget
	    || !AllManageableWidgetsInternal.Contains(Widget))
	{
		return;
	}

	AllManageableWidgetsInternal.RemoveSwap(Widget);
	FWidgetUtilsLibrary::DestroyWidget(*Widget);
}

/*********************************************************************************************
 * Core Widgets Initialization
 ********************************************************************************************* */

// Will try to start the process of initializing all widgets used in game
void UWidgetsSubsystem::TryInitWidgets()
{
	if (UUtilsLibrary::IsViewportInitialized())
	{
		InitWidgets();
	}
	else if (!FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewportResizedWhenInit);
	}
}

// Create and set widget objects once
void UWidgetsSubsystem::InitWidgets()
{
	if (AreWidgetInitialized())
	{
		return;
	}

	const UUIDataAsset& UIDataAsset = UUIDataAsset::Get();

	HUDWidgetInternal = CreateManageableWidgetChecked<UHUDWidget>(UIDataAsset.GetHUDWidgetClass());

	FPSCounterWidgetInternal = CreateManageableWidgetChecked(UIDataAsset.GetFPSCounterWidgetClass());

	SettingsWidgetInternal = CreateManageableWidgetChecked<USettingsWidget>(UIDataAsset.GetSettingsWidgetClass(), /*bAddToViewport*/true, /*ZOrder*/4);
	SettingsWidgetInternal->TryConstructSettings();

	static constexpr int32 MaxPlayersNum = 4;
	NicknameWidgetsInternal.Reserve(MaxPlayersNum);
	for (int32 Index = 0; Index < MaxPlayersNum; ++Index)
	{
		UPlayerName3DWidget* NicknameWidget = CreateManageableWidgetChecked<UPlayerName3DWidget>(UIDataAsset.GetNicknameWidgetClass(), /*bAddToViewport*/false);
		// Is drawn by 3D user widget component, no need add it to viewport
		NicknameWidgetsInternal.Emplace(NicknameWidget);
	}

	bAreWidgetInitializedInternal = true;

	if (OnWidgetsInitialized.IsBound())
	{
		OnWidgetsInitialized.Broadcast();
	}
}

// Removes all widgets and transient data
void UWidgetsSubsystem::CleanupWidgets()
{
	for (int32 Idx = AllManageableWidgetsInternal.Num() - 1; Idx >= 0; --Idx)
	{
		UUserWidget* It = AllManageableWidgetsInternal.IsValidIndex(Idx) ? AllManageableWidgetsInternal[Idx] : nullptr;
		if (It)
		{
			DestroyManageableWidget(It);
		}
	}

	AllManageableWidgetsInternal.Empty();
	NicknameWidgetsInternal.Empty();

	bAreWidgetInitializedInternal = false;
}

/*********************************************************************************************
 * Widgets Visibility
 ********************************************************************************************* */

// If true, changes all visible manageable widgets to hidden
void UWidgetsSubsystem::SetAllWidgetsVisibility(bool bMakeVisible)
{
	const ESlateVisibility DesiredVisibility = bMakeVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	const TArray<TObjectPtr<UUserWidget>>& WidgetsToProcess = bMakeVisible ? AllHiddenWidgetsInternal : AllManageableWidgetsInternal;

	if (!bMakeVisible)
	{
		AllHiddenWidgetsInternal.Empty();
	}

	for (TObjectPtr<UUserWidget> Widget : WidgetsToProcess)
	{
		if (Widget && Widget->IsVisible() != bMakeVisible)
		{
			Widget->SetVisibility(DesiredVisibility);

			if (!bMakeVisible)
			{
				AllHiddenWidgetsInternal.Add(Widget);
			}
		}
	}

	if (bMakeVisible)
	{
		AllHiddenWidgetsInternal.Empty();
	}
}

/*********************************************************************************************
 * FPS Counter
 ********************************************************************************************* */

// Set true to show the FPS counter widget on the HUD
void UWidgetsSubsystem::SetFPSCounterEnabled(bool bEnable)
{
	if (FPSCounterWidgetInternal)
	{
		const ESlateVisibility NewVisibility = bEnable ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		FPSCounterWidgetInternal->SetVisibility(NewVisibility);
		bIsFPSCounterEnabledInternal = bEnable;
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Callback for when the player controller is changed on this subsystem's owning local player
void UWidgetsSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	const AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayerController);
	if (!MyPC
	    || MyPC->bIsDebugCameraEnabledInternal)
	{
		// Do not initialize widgets if different controller is possessed, likely Debug Controller, or Debug Camera is enabled
		return;
	}

	if (AreWidgetInitialized())
	{
		// New player controller is set, likely level was changed, so perform cleanup first
		CleanupWidgets();
	}

	TryInitWidgets();
}

// Is called when this Subsystem is removed
void UWidgetsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	CleanupWidgets();
}

// Is called right after the game was started and windows size is set
void UWidgetsSubsystem::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	InitWidgets();
}