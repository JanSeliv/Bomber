// Fill out your copyright notice in the Description page of Project Settings.

#include "MyHUD.h"
#include "Bomber.h"
#include "InGameUserWidget.h"

// Sets default values for this HUD's properties
AMyHUD::AMyHUD()
{

	// ___ Create UMG widget
	TSubclassOf<UUserWidget> umgMenuClass, umgHudClass, umgCurrentClass;
	// Find UMenuUserWidgetUMG
	static ConstructorHelpers::FClassFinder<UUserWidget> umgMenuFinder(TEXT("/Game/Bomber/HUDs/Menu/UI_Menu"));
	if (umgMenuFinder.Succeeded()) // Check to make sure the blueprint widget HP_bar was actually found
		umgMenuClass = umgMenuFinder.Class;
	// Find UInGameUserWidgetUMG
	static ConstructorHelpers::FClassFinder<UUserWidget> umgHudFinder(TEXT("/Game/Bomber/HUDs/Hud/UI_HUD"));
	if (umgHudFinder.Succeeded()) // Check to make sure the blueprint widget was actually found
		umgHudClass = umgHudFinder.Class;
	// Select one of widgets
	umgCurrentClass = (UGameplayStatics::GetCurrentLevelName(GetWorld()) == "MenuLevel" ? umgMenuClass : umgHudClass);
	umgCurrentObj = CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), umgCurrentClass);
	//___ ___
}

void AMyHUD::BeginPlay()
{
	//Add to viewport selected UMG
	if (ISVALID(umgCurrentObj))
	{
		umgCurrentObj->AddToViewport();
	}
}
