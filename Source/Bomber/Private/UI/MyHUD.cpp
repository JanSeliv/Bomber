// Copyright 2021 Yevhenii Selivanov.

#include "UI/MyHUD.h"
//---
#include "GameFramework/MyGameUserSettings.h"
#include "Globals/SingletonLibrary.h"
#include "UI/InGameWidget.h"

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

	// Widget creating and adding it to viewport
	InGameWidget = CreateWidget<UInGameWidget>(GetOwningPlayerController(), UUIDataAsset::Get().GetInGameClass());
	if (InGameWidget) // successfully created
	{
		InGameWidget->AddToViewport();
	}

	UMyGameUserSettings::Get().InitSettings();
}
