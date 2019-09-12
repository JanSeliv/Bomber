// Copyright 2019 Yevhenii Selivanov.

#include "MyHUD.h"
#include "Bomber.h"
#include "ConstructorHelpers.h"
#include "UserWidget.h"

// Sets default values for this HUD's properties
AMyHUD::AMyHUD()
{
	// Find Menu Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> UmgMenuClassFinder(TEXT("/Game/Bomber/UI/Menu/UI_MenuWidget"));
	if (UmgMenuClassFinder.Succeeded())
	{
		UmgMenuClass = UmgMenuClassFinder.Class;
	}

	// Find In Game User Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> UmgLevelClassFinder(TEXT("/Game/Bomber/UI/InGame/UI_InGameWidget"));
	if (UmgLevelClassFinder.Succeeded())
	{
		UmgLevelClass = UmgLevelClassFinder.Class;
	}
}

// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	// Widget creating and adding it to viewport
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	const TSubclassOf<UUserWidget> UmgCurrentClass = CurrentLevelName == "MenuLevel" ? UmgMenuClass : UmgLevelClass;
	CreatedWidget = CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), UmgCurrentClass);
	if (IsValid(CreatedWidget))  // successfully created
	{
		CreatedWidget->AddToViewport();
	}
}
