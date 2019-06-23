// Fill out your copyright notice in the Description page of Project Settings.

#include "MyHUD.h"
#include "Bomber.h"
#include "ConstructorHelpers.h"
#include "InGameUserWidget.h"

// Sets default values for this HUD's properties
AMyHUD::AMyHUD()
{
	// ___ Create UMG widget
	TSubclassOf<UUserWidget> UmgMenuClass, UmgHudClass;
	// Find UMenuUserWidgetUMG
	static ConstructorHelpers::FClassFinder<UUserWidget> umgMenuFinder(TEXT("/Game/Bomber/HUDs/Menu/UI_Menu"));
	if (umgMenuFinder.Succeeded())  // Check to make sure the blueprint widget HP_bar was actually found
		UmgMenuClass = umgMenuFinder.Class;
	// Find UInGameUserWidgetUMG
	static ConstructorHelpers::FClassFinder<UUserWidget> umgHudFinder(TEXT("/Game/Bomber/HUDs/Hud/UI_HUD"));
	if (umgHudFinder.Succeeded())  // Check to make sure the blueprint widget was actually found
		UmgHudClass = umgHudFinder.Class;

	// Select one of widgets
	const TSubclassOf<UUserWidget> UmgCurrentClass = (UGameplayStatics::GetCurrentLevelName(AActor::GetWorld()) == "MenuLevel"
														  ? UmgMenuClass
														  : UmgHudClass);
	UmgCurrentObj = CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(AActor::GetWorld(), 0), UmgCurrentClass);
	//___ ___
}

void AMyHUD::BeginPlay()
{
	//Add to viewport selected UMG
	if (IS_VALID(UmgCurrentObj))
	{
		UmgCurrentObj->AddToViewport();
	}
}
