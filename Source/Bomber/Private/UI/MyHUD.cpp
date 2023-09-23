// Copyright (c) Yevhenii Selivanov.

#include "UI/MyHUD.h"
//---
#include "DataAssets/UIDataAsset.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "UI/InGameWidget.h"
#include "UI/SettingsWidget.h"
//---
#include "UnrealClient.h"
#include "Blueprint/UserWidget.h"
#include "Components/GameFrameworkComponentManager.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyHUD)

// Default constructor
AMyHUD::AMyHUD()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Go back input for UI widgets
void AMyHUD::BroadcastOnClose()
{
	if (OnClose.IsBound())
	{
		OnClose.Broadcast();
	}
}

// Set true to show the FPS counter widget on the HUD
void AMyHUD::SetFPSCounterEnabled(bool bEnable)
{
	if (FPSCounterWidgetInternal)
	{
		const ESlateVisibility NewVisibility = bEnable ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		FPSCounterWidgetInternal->SetVisibility(NewVisibility);
		bIsFPSCounterEnabledInternal = bEnable;
	}
}

// Init all widgets on gameplay starting before begin play
void AMyHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register HUD to let modular widgets to be dynamically added there
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	if (PlayerOwner
	    && !PlayerOwner->MyHUD)
	{
		PlayerOwner->MyHUD = this;
	}

	TryInitWidgets();
}

// Internal UUserWidget::CreateWidget wrapper
UUserWidget* AMyHUD::CreateWidgetByClass(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass, bool bAddToViewport/*= true*/, int32 ZOrder/* = 0*/)
{
	if (!ensureMsgf(PlayerController, TEXT("%s: 'PlayerController' is null"), *FString(__FUNCTION__))
	    || !ensureMsgf(WidgetClass, TEXT("%s: 'WidgetClass' is null"), *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	UUserWidget* CreatedWidget = CreateWidget(PlayerController, WidgetClass);
	checkf(CreatedWidget, TEXT("%s: ERROR: %s failed to create"), *FString(__FUNCTION__), *WidgetClass->GetName());

	if (bAddToViewport)
	{
		CreatedWidget->AddToViewport(ZOrder);
	}

	return CreatedWidget;
}

// Will try to start the process of initializing all widgets used in game
void AMyHUD::TryInitWidgets()
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
void AMyHUD::InitWidgets()
{
	if (AreWidgetInitialized())
	{
		return;
	}

	const UUIDataAsset& UIDataAsset = UUIDataAsset::Get();

	InGameWidgetInternal = CreateWidgetByClass<UInGameWidget>(UIDataAsset.GetInGameWidgetClass());

	FPSCounterWidgetInternal = CreateWidgetByClass(UIDataAsset.GetFPSCounterWidgetClass());

	SettingsWidgetInternal = CreateWidgetByClass<USettingsWidget>(UIDataAsset.GetSettingsWidgetClass(), /*bAddToViewport*/true, /*ZOrder*/4);
	SettingsWidgetInternal->TryConstructSettings();

	static constexpr int32 MaxPlayersNum = 4;
	NicknameWidgetsInternal.Reserve(MaxPlayersNum);
	for (int32 Index = 0; Index < MaxPlayersNum; ++Index)
	{
		UUserWidget* NicknameWidget = CreateWidgetByClass(UIDataAsset.GetNicknameWidgetClass(), /*bAddToViewport*/false); // Is drawn by 3D user widget component, no need add it to viewport
		NicknameWidgetsInternal.Emplace(NicknameWidget);
	}

	bAreWidgetInitializedInternal = true;

	if (OnWidgetsInitialized.IsBound())
	{
		OnWidgetsInitialized.Broadcast();
	}
}

// Is called right after the game was started and windows size is set
void AMyHUD::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	InitWidgets();
}
