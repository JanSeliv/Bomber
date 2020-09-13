// Copyright 2020 Yevhenii Selivanov.

#include "MyHUD.h"
//---
#include "ConstructorHelpers.h"
#include "UserWidget.h"

// Sets default values for this HUD's properties
AMyHUD::AMyHUD()
{
	// Find In Game User Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> UmgLevelClassFinder(TEXT("/Game/Bomber/UI/InGame/WBP_InGame"));
	if (UmgLevelClassFinder.Succeeded())
	{
		InGameWidgetClass = UmgLevelClassFinder.Class;
	}
}


// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	// Widget creating and adding it to viewport
	InGameWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), InGameWidgetClass);
	if (InGameWidget) // successfully created
	{
		InGameWidget->AddToViewport();
	}
}

