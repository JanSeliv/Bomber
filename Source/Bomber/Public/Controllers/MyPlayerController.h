// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"
//---
#include "MyPlayerController.generated.h"

enum class ECurrentGameState : uint8;

class UMyInputMappingContext;

/**
 * The player controller class.
 * @see Access its data with UPlayerInputDataAsset (Content/Bomber/DataAssets/DA_PlayerInput).
 */
UCLASS()
class BOMBER_API AMyPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	/** Sets default values for this controller's properties. */
	AMyPlayerController();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPossessed, class APlayerCharacter*, PlayerCharacter);

	/** Notifies the server and clients when this controller possesses new player character. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnPossessed OnPossessed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetPlayerState, class AMyPlayerState*, MyPlayerState);

	/** Notifies the server and clients when the player state is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnSetPlayerState OnSetPlayerState;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateCreated, class AMyGameStateBase*, MyGameState);

	/** Notifies the server and clients when the game state is initialized. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnGameStateCreated OnGameStateCreated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSetupPlayerInputs);

	/** Called when controller binds player inputs to notify other systems that player controller is ready to bind their own input actions. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnSetupPlayerInputs OnSetupPlayerInputs;

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
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

	/*********************************************************************************************
	 * Enhanced Input
	 ********************************************************************************************* */
public:
	/** Returns the Enhanced Input Local Player Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Returns the Enhanced Input Component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UEnhancedInputComponent* GetEnhancedInputComponent() const;

	/** Returns the Enhanced Player Input. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UEnhancedPlayerInput* GetEnhancedPlayerInput() const;

	/** Set up input bindings in given contexts.
	 * @param InputContexts Contexts to bind input actions.
	 * @param bClearPreviousBindings If true, all previous bindings will be removed. */
	void BindInputActionsInContexts(const TArray<const UMyInputMappingContext*>& InputContexts, bool bClearPreviousBindings = false);

	/** Adds input contexts to the list to be auto turned of or on according current game state.
	 * Make sure UMyInputMappingContext::ActiveForStatesInternal is set.
	 * @param InputContexts Contexts to manage.
	 * @see AMyPlayerController::AllInputContextsInternal */
	void AddInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** List of all input contexts to be auto turned of or on according current game state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "All Input Contexts"))
	TArray<TObjectPtr<const UMyInputMappingContext>> AllInputContextsInternal;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** This is called only in the gameplay before calling begin play. */
	virtual void PostInitializeComponents() override;

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

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Set up custom input bindings. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetupPlayerInputs();

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

	/*********************************************************************************************
	 * Input Contexts management
	 ********************************************************************************************* */
public:
	/** Returns true if specified input context is enabled.
	 * @param InputContext Context to verify. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	bool IsInputContextEnabled(const UMyInputMappingContext* InputContext) const;

	/** Enables or disables specified input context.
	 * @param bEnable set true to add specified input context, otherwise it will be removed from local player.
	 * @param InputContext Context to set. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InputContext);

	/** Takes all cached inputs contexts and turns them on or off according given game state.
	 * @param bEnable If true, all matching contexts will be enabled. If false, all matching contexts will be disabled.
	 * @param CurrentGameState Game state to check matching.
	 * @param bInvertRest If true, all other not matching contexts will be toggled to the opposite of given state (!bEnable).
	 * @see AMyPlayerController::AddInputContexts */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetInputContextsEnabled(bool bEnable, ECurrentGameState CurrentGameState, bool bInvertRest = false);

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
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
