// Copyright 2020 Yevhenii Selivanov.

#include "MyHUD.h"
//---
#include "Bomber.h"
//---
#include "ConstructorHelpers.h"
#include "SingletonLibrary.h"
#include "UserWidget.h"

// Sets default values for this HUD's properties
AMyHUD::AMyHUD()
{
	// Find Menu Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> UmgMenuClassFinder(TEXT("/Game/Bomber/UI/Menu/WBP_Menu"));
	if (UmgMenuClassFinder.Succeeded())
	{
		UmgMenuClass = UmgMenuClassFinder.Class;
	}

	// Find In Game User Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> UmgLevelClassFinder(TEXT("/Game/Bomber/UI/InGame/WBP_InGame"));
	if (UmgLevelClassFinder.Succeeded())
	{
		UmgLevelClass = UmgLevelClassFinder.Class;
	}
}

// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	// Widget creating and adding it to viewport

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	const TSubclassOf<UUserWidget> UmgCurrentClass =
		CurrentLevelName == USingletonLibrary::GetMenuLevelName()  // is menu level
			? UmgMenuClass
			: UmgLevelClass;
	CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), UmgCurrentClass);
	if (IsValid(CreatedWidget))	 // successfully created
	{
		CreatedWidget->AddToViewport();
	}
}
