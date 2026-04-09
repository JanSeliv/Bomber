// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

#include "NewMainMenuWidget.generated.h"

class UButton;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> PlayButton = nullptr;

	/** The button to choose next player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> NextPlayerButton = nullptr;

	/** The button to choose previous player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> PrevPlayerButton = nullptr;

	/** The button to switch the skin of chosen player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> NextSkinButton = nullptr;

	/** The button to open the Settings Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> SettingsButton = nullptr;

	/** The button to quit the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> QuitGameButton = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the widget is removed from the viewport. */
	virtual void NativeDestruct() override;

	/** Is called when player pressed the button to start the game. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnPlayButtonPressed();

	/** Is called when player pressed the button to choose next player. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNextPlayerButtonPressed();

	/** Is called when player pressed the button to choose previous player. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnPrevPlayerButtonPressed();

	/** Sets the preview mesh of a player depending on specified incrementer.
	 * @param Incrementer 1 set the next player, -1 set previous. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SwitchCurrentPlayer(int32 Incrementer);

	/** Sets the next skin in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void OnNextSkinButtonPressed();

	/** Is called when player pressed the button to open the Settings. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void OnSettingsButtonPressed();

	/** Is called when player pressed the button to quit the game. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void OnQuitGameButtonPressed();
};
