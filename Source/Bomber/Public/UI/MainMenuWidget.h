// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Blueprint/UserWidget.h"
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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void ChooseRight();

	/** Sets the previous player in the Menu. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void ChooseLeft();

	/** Sets the next level in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseForward();

	/** Sets the previous level in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ChooseBack();

	/** Sets the next skin in the Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void NextSkin();

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
	class ACarousel* MainMenuActorInternal; //[G]

	/** Sets the level depending on specified incrementer.
	 * @param Incrementer if +1 will set the next level, if -1 will set previous. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SwitchCurrentLevel(int32 Incrementer);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
