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
	 * Game States
	 ********************************************************************************************* */
public:
	/** Returns true if current game state can be eventually changed. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool CanChangeGameState(ECurrentGameState NewGameState) const;

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (DisplayName = "Set Game State"))
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Sets the GameStarting game state. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetGameStartingState();

	/** Sets the Menu game state. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMenuState();

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** List of all input contexts to be auto turned of or on according current game state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "All Input Contexts"))
	TArray<TObjectPtr<const UMyInputMappingContext>> AllInputContextsInternal;

	/** Component that responsible for mouse-related logic like showing and hiding itself. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = ""))
	TObjectPtr<class UMouseActivityComponent> MouseComponentInternal = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** Locks or unlocks movement input, is declared in parent as UFUNCTION.
	 * @param bShouldIgnore	If true, move input is ignored. If false, input is not ignored.*/
	virtual void SetIgnoreMoveInput(bool bShouldIgnore) override;

protected:
	/** This is called only in the gameplay before calling begin play. */
	virtual void PostInitializeComponents() override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Is overriden to notify when this controller possesses new player character.
	 * @param InPawn The Pawn to be possessed. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Is overriden to notify the client when this controller possesses new player character. */
	virtual void OnRep_Pawn() override;

	/** Is overridden to spawn player state or reuse existing one. */
	virtual void InitPlayerState() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called when all game widgets are initialized. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnWidgetsInitialized();

	/** Listen to toggle movement input. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Listens to handle input on opening and closing the Settings widget. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnToggledSettings(bool bIsVisible);

	/*********************************************************************************************
	 * Inputs management
	 ********************************************************************************************* */
public:
	/** Returns true if Player Controller is ready to setup all the inputs. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool CanBindInputActions() const;

	/** Adds given contexts to the list of auto managed and binds their input actions . */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetupInputContexts(const TArray<UMyInputMappingContext*>& InputContexts);
	void SetupInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts);
	void RemoveInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts);

	/** Prevents built-in slate input on UMG. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DisplayName = "Set UI Input Ignored"))
	void SetUIInputIgnored();

	/** Takes all cached inputs contexts and turns them on or off according given game state.
	 * @param bEnable If true, all matching contexts will be enabled. If false, all matching contexts will be disabled.
	 * @param CurrentGameState Game state to check matching.
	 * @param bInvertRest If true, all other not matching contexts will be toggled to the opposite of given state (!bEnable).
	 * @see AMyPlayerController::AddInputContexts */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetAllInputContextsEnabled(bool bEnable, ECurrentGameState CurrentGameState, bool bInvertRest = false);

	/** Enables or disables specified input context.
	 * @param bEnable If true, the context will be enabled. If false, the context will be disabled.
	 * @param InInputContext The input context to enable or disable. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InInputContext);

	/** Set up input bindings in given contexts. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindInputActionsInContext(const UMyInputMappingContext* InInputContext);

	/** Adds input contexts to the list to be auto turned of or on according current game state.
	 * Make sure UMyInputMappingContext::ActiveForStatesInternal is set.
	 * @param InputContexts Contexts to manage.
	 * @see AMyPlayerController::AllInputContextsInternal */
	void AddNewInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts);

	/** Returns the component that responsible for mouse-related logic like showing and hiding itself. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMouseActivityComponent* GetMouseActivityComponent() const { return MouseComponentInternal; }
};