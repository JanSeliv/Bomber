// Copyright (c) Yevhenii Selivanov.

#include "UI/MyHUD.h"
//---
#include "UtilsLibrary.h"
#include "Globals/DataAssetsContainer.h"
#include "UtilityLibraries/SingletonLibrary.h"
#include "UI/InGameWidget.h"
#include "UI/MainMenuWidget.h"
#include "UI/SettingsWidget.h"

// Returns the UI data asset
const UUIDataAsset& UUIDataAsset::Get()
{
	const UUIDataAsset* UIDataAsset = UDataAssetsContainer::GetUIDataAsset();
	checkf(UIDataAsset, TEXT("The UI Data Asset is not valid"));
	return *UIDataAsset;
}

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

	if (PlayerOwner
	    && !PlayerOwner->MyHUD)
	{
		PlayerOwner->MyHUD = this;
	}

	TryInitWidgets();
}

// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();
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

	APlayerController* PlayerController = PlayerOwner.Get();
	if (!ensureMsgf(PlayerController, TEXT("ASSERT: 'PlayerController' is not valid")))
	{
		return;
	}

	const UUIDataAsset& UIDataAsset = UUIDataAsset::Get();

	const TSubclassOf<UMainMenuWidget>& MainMenuWidgetClass = UIDataAsset.GetMainMenuWidgetClass();
	if (ensureMsgf(MainMenuWidgetClass, TEXT("ASSERT: 'InGameWidgetClass' is not set in the UI data table")))
	{
		MainMenuWidgetInternal = CreateWidget<UMainMenuWidget>(PlayerController, MainMenuWidgetClass);
		checkf(MainMenuWidgetInternal, TEXT("ERROR: MainMenuWidgetInternal failed to create"));
		// Main Menu widget is drawn by 3D user widget component, no need add it to viewport
	}

	const TSubclassOf<UInGameWidget>& InGameWidgetClass = UIDataAsset.GetInGameWidgetClass();
	if (ensureMsgf(InGameWidgetClass, TEXT("ASSERT: 'InGameWidgetClass' is not set in the UI data table")))
	{
		InGameWidgetInternal = CreateWidget<UInGameWidget>(PlayerController, InGameWidgetClass);
		checkf(InGameWidgetInternal, TEXT("ERROR: InGameWidgetInternal failed to create"));
		InGameWidgetInternal->AddToViewport();
	}

	const TSubclassOf<USettingsWidget>& SettingsWidgetClass = UIDataAsset.GetSettingsWidgetClass();
	if (ensureMsgf(SettingsWidgetClass, TEXT("ASSERT: 'SettingsWidgetClass' is not set in the UI data table")))
	{
		SettingsWidgetInternal = CreateWidget<USettingsWidget>(PlayerController, SettingsWidgetClass);
		checkf(SettingsWidgetInternal, TEXT("ERROR: SettingsWidgetInternal failed to create"));
		SettingsWidgetInternal->AddToViewport();
	}

	const TSubclassOf<UUserWidget>& FPSCounterWidgetClass = UIDataAsset.GetFPSCounterWidgetClass();
	if (ensureMsgf(FPSCounterWidgetClass, TEXT("ASSERT: 'FPSCounterWidgetClass' is not set in the UI data table")))
	{
		FPSCounterWidgetInternal = CreateWidget<UUserWidget>(PlayerController, FPSCounterWidgetClass);
		checkf(FPSCounterWidgetInternal, TEXT("ERROR: FPSCounterWidgetInternal failed to create"));
		FPSCounterWidgetInternal->AddToViewport();
	}

	const TSubclassOf<UUserWidget>& NicknameWidgetClass = UIDataAsset.GetNicknameWidgetClass();
	if (ensureMsgf(NicknameWidgetClass, TEXT("ASSERT: 'NicknameWidgetClass' is not set in the UI data table")))
	{
		static constexpr int32 MaxPlayersNum = 4;
		NicknameWidgetsInternal.Reserve(MaxPlayersNum);
		for (int32 Index = 0; Index < MaxPlayersNum; ++Index)
		{
			UUserWidget* NicknameWidget = CreateWidget(PlayerController, NicknameWidgetClass);
			checkf(NicknameWidget, TEXT("ERROR: NicknameWidget failed to create"));
			NicknameWidgetsInternal.Emplace(NicknameWidget);
		}
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
