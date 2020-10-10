// Copyright 2020 Yevhenii Selivanov.

#include "MyHUD.h"
//---
#include "InGameWidget.h"
#include "SingletonLibrary.h"

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

