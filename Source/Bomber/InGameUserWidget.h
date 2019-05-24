// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "InGameUserWidget.generated.h"

/**
 *
 */
UCLASS(Abstract)
class BOMBER_API UInGameUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct() override;

	//update appearance dynamically in the editor
	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++")
		void ShowInGameState();

	/*
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		class UTextBlock* test;
	*/

};
