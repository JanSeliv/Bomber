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

// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	const UUIDataAsset& UIDataAsset = UUIDataAsset::Get();

	InGameWidgetInternal = CreateWidget<UInGameWidget>(PlayerOwner, UIDataAsset.GetInGameWidgetClass());
	if (InGameWidgetInternal)
	{
		InGameWidgetInternal->AddToViewport();
	}

	SettingsWidgetInternal = CreateWidget<USettingsWidget>(PlayerOwner, UIDataAsset.GetSettingsWidgetClass());
	if (SettingsWidgetInternal)
	{
		SettingsWidgetInternal->AddToViewport();
	}
}
