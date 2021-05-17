// Copyright 2021 Yevhenii Selivanov.

#include "UI/MyHUD.h"
//---
#include "Globals/SingletonLibrary.h"
#include "UI/InGameWidget.h"
#include "UI/SettingsWidget.h"

// Returns the UI data asset
const UUIDataAsset& UUIDataAsset::Get()
{
	const UUIDataAsset* UIDataAsset = USingletonLibrary::GetUIDataAsset();
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
void AMyHUD::GoUIBack()
{
	if (OnGoUIBack.IsBound())
	{
		OnGoUIBack.Broadcast();
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

	InitWidgets();
}

// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();
}

// Create and set widget objects once
void AMyHUD::InitWidgets()
{
	if (!ensureMsgf(PlayerOwner, TEXT("ASSERT: 'PlayerOwner' is not valid")))
	{
		return;
	}

	const UUIDataAsset& UIDataAsset = UUIDataAsset::Get();

	const TSubclassOf<UInGameWidget>& InGameWidgetClass = UIDataAsset.GetInGameWidgetClass();
	if (ensureMsgf(InGameWidgetClass, TEXT("ASSERT: 'InGameWidgetClass' is not set in the UI data table")))
	{
		InGameWidgetInternal = CreateWidget<UInGameWidget>(PlayerOwner, InGameWidgetClass);
		checkf(InGameWidgetInternal, TEXT("ERROR: InGameWidgetInternal failed to create"));
		InGameWidgetInternal->AddToViewport();
	}

	const TSubclassOf<USettingsWidget>& SettingsWidgetClass = UIDataAsset.GetSettingsWidgetClass();
	if (ensureMsgf(SettingsWidgetClass, TEXT("ASSERT: 'SettingsWidgetClass' is not set in the UI data table")))
	{
		SettingsWidgetInternal = CreateWidget<USettingsWidget>(PlayerOwner, SettingsWidgetClass);
		checkf(SettingsWidgetInternal, TEXT("ERROR: SettingsWidgetInternal failed to create"));
		SettingsWidgetInternal->AddToViewport();
	}

	const TSubclassOf<UUserWidget>& FPSCounterWidgetClass = UIDataAsset.GetFPSCounterWidgetClass();
	if (ensureMsgf(FPSCounterWidgetClass, TEXT("ASSERT: 'FPSCounterWidgetClass' is not set in the UI data table")))
	{
		FPSCounterWidgetInternal = CreateWidget<UUserWidget>(PlayerOwner, FPSCounterWidgetClass);
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
			UUserWidget* NicknameWidget = CreateWidget(PlayerOwner, NicknameWidgetClass);
			checkf(NicknameWidget, TEXT("ERROR: NicknameWidget failed to create"));
			NicknameWidgetsInternal.Emplace(NicknameWidget);
		}
	}
}
