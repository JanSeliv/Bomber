// Copyright 2020 Yevhenii Selivanov.

#include "UI/MyHUD.h"
//---
#include "Globals/SingletonLibrary.h"
#include "UI/InGameWidget.h"

// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	// Widget creating and adding it to viewport
	if (const UUIDataAsset* UIDataAsset = USingletonLibrary::GetUIDataAsset())
	{
		InGameWidget = CreateWidget<UInGameWidget>(GetOwningPlayerController(), UIDataAsset->GetInGameClass());
		if (InGameWidget) // successfully created
		{
			InGameWidget->AddToViewport();
		}
	}
}
