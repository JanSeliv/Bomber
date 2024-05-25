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

// Callback for when the player controller is changed on this subsystem's owning local player
void UWidgetsSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	if (NewPlayerController
	    && !AreWidgetInitialized())
	{
		TryInitWidgets();
	}
}

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

	HUDWidgetInternal = FWidgetUtilsLibrary::CreateWidgetChecked<UHUDWidget>(UIDataAsset.GetHUDWidgetClass());

	FPSCounterWidgetInternal = FWidgetUtilsLibrary::CreateWidgetByClass(UIDataAsset.GetFPSCounterWidgetClass());

	SettingsWidgetInternal = FWidgetUtilsLibrary::CreateWidgetChecked<USettingsWidget>(UIDataAsset.GetSettingsWidgetClass(), /*bAddToViewport*/true, /*ZOrder*/4);
	SettingsWidgetInternal->TryConstructSettings();

	static constexpr int32 MaxPlayersNum = 4;
	NicknameWidgetsInternal.Reserve(MaxPlayersNum);
	for (int32 Index = 0; Index < MaxPlayersNum; ++Index)
	{
		UPlayerName3DWidget* NicknameWidget = FWidgetUtilsLibrary::CreateWidgetChecked<UPlayerName3DWidget>(UIDataAsset.GetNicknameWidgetClass(), /*bAddToViewport*/false);
		// Is drawn by 3D user widget component, no need add it to viewport
		NicknameWidgetsInternal.Emplace(NicknameWidget);
	}

	bAreWidgetInitializedInternal = true;

	if (OnWidgetsInitialized.IsBound())
	{
		OnWidgetsInitialized.Broadcast();
	}
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