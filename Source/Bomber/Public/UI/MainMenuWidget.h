// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "Bomber.h"
#include "InputActionValue.h"
//---
#include "MainMenuWidget.generated.h"

/**
 * Main menu user widget.
 */
UCLASS()
class UMainMenuWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainMenuReady);

	/** Is called to notify listeners is Main Menu is created and ready to be shown. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnMainMenuReady OnMainMenuReady;

	/** Initializes the main menu widget.
	 * @param InMainMenuActor Sets the Main Menu actor on the scene. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void InitMainMenuWidget(class ACarousel* InMainMenuActor);

	/** Returns true if the Main Menu is ready. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool IsReadyMainMenu() const { return MainMenuActorInternal != nullptr; }

	/** Returns the Main Menu actor on the scene. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class ACarousel* GetMainMenuActor() const { return MainMenuActorInternal; }

	/** Sets the next player in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseRight();

	/** Sets the previous player in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseLeft();

	/** Sets the previous or next player in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseRightLeft(const FInputActionValue& ActionValue);
	
	/** Sets the next level in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseForward();

	/** Sets the previous level in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseBack();

	/** Sets the previous or next level in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseBackForward(const FInputActionValue& ActionValue);
	
	/** Sets the next skin in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void NextSkin();

	/** Sets the chosen on UI the level type. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseNewLevel(ELevelType LevelType);

	/** Is executed when player pressed the button of starting the game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StartGame();

	/** Is executed when player decided to close the game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void QuitGame();

protected:
	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/* Updates appearance dynamically in the editor. */
	virtual void SynchronizeProperties() override;

	/** The Main Menu actor on the scene.  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main Menu Actor"))
	TObjectPtr<class ACarousel> MainMenuActorInternal = nullptr; //[G]

	/** Sets the level depending on specified incrementer.
	 * @param Incrementer 1 set the next level, -1 set previous. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SwitchCurrentLevel(int32 Incrementer);

	/** Sets the preview mesh of a player depending on specified incrementer.
	 * @param Incrementer 1 set the next player, -1 set previous. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SwitchCurrentPlayer(int32 Incrementer);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
