// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "NewMainMenuWidget.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Displays the Main Menu.
 */
UCLASS()
class NEWMAINMENU_API UNewMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** The button to allow player go back to the Main Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> PlayButton = nullptr;

	/** The button to choose next player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> NextPlayerButton = nullptr;

	/** The button to choose previous player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> PrevPlayerButton = nullptr;

	/** The button to switch the skin of chosen player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> NextSkinButton = nullptr;

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

	/** Is called when player pressed the button to start the game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayButtonPressed();

	/** Is called when player pressed the button to choose next player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnNextPlayerButtonPressed();

	/** Is called when player pressed the button to choose previous player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPrevPlayerButtonPressed();

	/** Sets the preview mesh of a player depending on specified incrementer.
	* @param Incrementer 1 set the next player, -1 set previous. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SwitchCurrentPlayer(int32 Incrementer);

	/** Sets the next skin in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnNextSkinButtonPressed();
};
