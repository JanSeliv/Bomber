// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
//---
#include "Bomber.h"
#include "Globals//LevelActorDataAsset.h"
//---
#include "MyPlayerController.generated.h"

/**
* Contains all data that describe player input.
*/
UCLASS(Blueprintable, BlueprintType)
class UPlayerInputDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the player input data asset. */
	static const UPlayerInputDataAsset& Get();

	/** Returns all input contexts contained in this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetAllInputContexts(TArray<class UMyInputMappingContext*>& OutInputContexts) const;

	/** Returns the Enhanced Input Mapping Context of gameplay actions for specified local player.
	* @param LocalPlayerIndex The index of a local player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UMyInputMappingContext* GetGameplayInputContext(int32 LocalPlayerIndex) const;

	/** Returns the Enhanced Input Mapping Context of actions on the User Interface.
	* @see UPlayerInputDataAsset::MainMenuInputContextInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UMyInputMappingContext* GetMainMenuInputContext() const { return MainMenuInputContextInternal; }

	/** Returns the Enhanced Input Mapping Context of actions on the User Interface.
	  * @see UPlayerInputDataAsset:: */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UMyInputMappingContext* GetInGameMenuInputContext() const { return InGameMenuInputContextInternal; }

protected:
	/** Enhanced Input Mapping Contexts of gameplay actions for local players
	 *  Are selectable classes instead of objects directly to avoid changing data asset by MapKey\UnmapKey. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Gameplay Input Context Classes", ShowOnlyInnerProperties))
	TArray<TSubclassOf<UMyInputMappingContext>> GameplayInputContextClassesInternal; //[D]

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main Menu Input Context", ShowOnlyInnerProperties))
	TObjectPtr<class UMyInputMappingContext> MainMenuInputContextInternal; //[D]

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In-Game Menu Input Context", ShowOnlyInnerProperties))
	TObjectPtr<class UMyInputMappingContext> InGameMenuInputContextInternal; //[D]

private:
	/** Are created dynamically by specified input classes.
	 * @see UPlayerInputDataAsset::GameplayInputContextClassesInternal */
	UPROPERTY(Transient)
	mutable TArray<TObjectPtr<class UMyInputMappingContext>> GameplayInputContextsInternal; //[G]
};

/**
 * The player controller class
 */
UCLASS()
class AMyPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	/** Sets default values for this controller's properties. */
	AMyPlayerController();

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (DisplayName = "Set Game State"))
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Returns true if the mouse cursor can be hidden. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	bool CanHideMouse() const;

	/** Called to to set the mouse cursor visibility.
	 * @param bShouldShow true to show mouse cursor, otherwise hide it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseVisibility(bool bShouldShow);

	/** Returns the Enhanced Input Local Player Subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Returns the Enhanced Input Component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UEnhancedInputComponent* GetEnhancedInputComponent() const;

	/** Returns the Enhanced Player Input. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UEnhancedPlayerInput* GetEnhancedPlayerInput() const;

protected:
	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Locks or unlocks movement input, is declared in parent as UFUNCTION.
	* @param bShouldIgnore	If true, move input is ignored. If false, input is not ignored.*/
	virtual void SetIgnoreMoveInput(bool bShouldIgnore) override;

	/** Overridable native function for when this controller unpossesses its pawn. */
	virtual void OnUnPossess() override;

	/** Set up custom input bindings. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindInputActions();

	/** Prevents built-in slate input on UMG. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DisplayName = "Set UI Input Ignored"))
	void SetUIInputIgnored();

	/** Listen to toggle movement input and mouse cursor. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Listens to handle input on opening and closing the InGame Menu widget. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnToggledInGameMenu(bool bIsVisible);

	/** Called default console commands on begin play. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ExecuteDefaultConsoleCommands();

	/** Enables or disables input contexts of gameplay input actions.
	 * @param bEnable set true to add gameplay input context, otherwise it will be removed from local player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetGameplayInputContextEnabled(bool bEnable);

	/** Returns true if specified input context is enabled.
	 * @param InputContext Context to verify. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	bool IsInputContextEnabled(const UMyInputMappingContext* InputContext) const;

	/** Enables or disables specified input context.
	 * @param bEnable set true to add specified input context, otherwise it will be removed from local player.
	 * @param InputContext Context to set. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InputContext);

	/** Move the player character by the forward vector. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void MoveUpDown(const FInputActionValue& ActionValue);

	/** Move the player character by the right vector. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void MoveRightLeft(const FInputActionValue& ActionValue);

	/** Executes spawning the bomb on controllable player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SpawnBomb();

	/** Sets the GameStarting game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetGameStartingState();

	/** Sets the Menu game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetMenuState();

	/** Is called when all game widgets are initialized. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnWidgetsInitialized();
};
