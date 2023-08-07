// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "NMMCinematicStateWidget.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Is active while game is in cinematic state, is responsible for skipping cinematic.
 */
UCLASS()
class NEWMAINMENU_API UNMMCinematicStateWidget : public UUserWidget
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Is called to skip cinematic. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SkipCinematic();

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Is called to start listening game state changes. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnGameStateChanged(class AMyGameStateBase* MyGameState);
};
