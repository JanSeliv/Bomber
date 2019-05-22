// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameUserWidget.h"
#include "Bomber.h"
#include "Components/TextBlock.h"

void UInGameUserWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

}


void UInGameUserWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	/*
	if (test)
	{
		test->SetText(FText::FromString("TEST2"));
	}
	*/
}


void UInGameUserWidget::ShowInGameState_Implementation(UObject* widget)
{
	
}