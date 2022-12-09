// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"
//---
#include "Bomber.h"
//---
#include "MyPlayerController.generated.h"

/**
 * The player controller class
 */
UCLASS()
class BOMBER_API AMyPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	/** Sets default values for this controller's properties. */
	AMyPlayerController();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPossessed, class APlayerCharacter*, PlayerCharacter);

	/** Notifies the server and clients when this controller possesses new player character. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnPossessed OnPossessed; //[DMD]

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetPlayerState, class AMyPlayerState*, MyPlayerState);

	/** Notifies the server and clients when the player state is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnSetPlayerState OnSetPlayerState; //[DMD]

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateCreated, class AMyGameStateBase*, MyGameState);

	/** Notifies the server and clients when the game state is initialized. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnGameStateCreated OnGameStateCreated; //[DMD]

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (DisplayName = "Set Game State"))
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Sets the GameStarting game state. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetGameStartingState();

	/** Sets the Menu game state. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMenuState();

	/** Returns true if the mouse cursor can be hidden. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool CanHideMouse() const;

	/** Called to to set the mouse cursor visibility.
	 * @param bShouldShow true to show mouse cursor, otherwise hide it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseVisibility(bool bShouldShow);

	/** If true, set the mouse focus on game and UI, otherwise only focusing on game inputs. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseFocusOnUI(bool bFocusOnUI);

	/** Returns the Enhanced Input Local Player Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Returns the Enhanced Input Component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UEnhancedInputComponent* GetEnhancedInputComponent() const;

	/** Returns the Enhanced Player Input. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UEnhancedPlayerInput* GetEnhancedPlayerInput() const;

protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Locks or unlocks movement input, is declared in parent as UFUNCTION.
	* @param bShouldIgnore	If true, move input is ignored. If false, input is not ignored.*/
	virtual void SetIgnoreMoveInput(bool bShouldIgnore) override;

	/** Is overriden to notify when this controller possesses new player character.
	 * @param InPawn The Pawn to be possessed. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Is overriden to notify the client when this controller possesses new player character. */
	virtual void OnRep_Pawn() override;

	/** Is overriden to notify the client when is set new player state. */
	virtual void OnRep_PlayerState() override;

	/** Set up custom input bindings. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindInputActions();

	/** Prevents built-in slate input on UMG. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DisplayName = "Set UI Input Ignored"))
	void SetUIInputIgnored();

	/** Listen to toggle movement input and mouse cursor. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Listens to handle input on opening and closing the InGame Menu widget. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnToggledInGameMenu(bool bIsVisible);

	/** Listens to handle input on opening and closing the Settings widget. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnToggledSettings(bool bIsVisible);

	/** Enables or disables input contexts of gameplay input actions.
	 * @param bEnable set true to add gameplay input context, otherwise it will be removed from local player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetGameplayInputContextEnabled(bool bEnable);

	/** Returns true if specified input context is enabled.
	 * @param InputContext Context to verify. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	bool IsInputContextEnabled(const class UMyInputMappingContext* InputContext) const;

	/** Enables or disables specified input context.
	 * @param bEnable set true to add specified input context, otherwise it will be removed from local player.
	 * @param InputContext Context to set. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetInputContextEnabled(bool bEnable, const class UMyInputMappingContext* InputContext);

	/** Is called when all game widgets are initialized. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnWidgetsInitialized();

	/** Is called on server and on client when the player state is set. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BroadcastOnSetPlayerState();

	/** Start listening creating the game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnGameStateCreated();

	/** Is called on server and on client when the game state is initialized. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BroadcastOnGameStateCreated(class AGameStateBase* GameState);
};
