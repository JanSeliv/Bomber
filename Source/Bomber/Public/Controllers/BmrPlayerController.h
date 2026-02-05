// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"

#include "BmrPlayerController.generated.h"

enum class EBmrCurrentGameState : uint8;

class UBmrInputMappingContext;

/**
 * The player controller class.
 * @see Access its data with UPlayerInputDataAsset (Content/Bomber/DataAssets/DA_PlayerInput).
 */
UCLASS()
class BOMBER_API ABmrPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	/** Sets default values for this controller's properties. */
	ABmrPlayerController();

	/*********************************************************************************************
	 * Server RPCs
	 ********************************************************************************************* */
public:
	/** Sends a gameplay event to the server via Gameplay Message Router, is useful for actions happening only by client such as UI buttons to start the game etc. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "[Bomber]")
	void ServerBroadcastMessage(const struct FGameplayEventData& Payload);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** List of all input contexts to be auto turned of or on according current game state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<TObjectPtr<const UBmrInputMappingContext>> AllInputContexts;

	/** Component that responsible for mouse-related logic like showing and hiding itself. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrMouseActivityComponent> MouseComponent = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** This is called only in the gameplay before calling begin play. */
	virtual void PostInitializeComponents() override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Is overriden to be used when Input System is initialized. */
	virtual void InitInputSystem() override;

	/** Is overriden to notify when this controller possesses new player character.
	 * @param InPawn The Pawn to be possessed. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Is overriden to notify the client when this controller possesses new player character. */
	virtual void OnRep_Pawn() override;

	/** Is overridden to spawn player state or reuse existing one. */
	virtual void InitPlayerState() override;

	/** Is overridden to prevent destroyed possessed pawn, which is expected to be reused. */
	virtual void PawnLeavingGame() override;

	/** Is overridden to stop the game state State Tree when entering Render Movie cinematic mode. */
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** Is overridden to perform cleanup of the controller when it is destroyed. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnyCinematicStarted, const UObject*, LevelSequence, const UObject*, FromInstigator);

	/** Is called only on local player on started watching an in-game cinematic. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnAnyCinematicStarted OnAnyCinematicStarted;

protected:
	/** Is called when all game widgets are initialized. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnWidgetsInitialized();

	/** Listen to toggle movement input. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Listens to handle input on opening and closing the Settings widget. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnToggledSettings(bool bIsVisible);

	/*********************************************************************************************
	 * Inputs management
	 ********************************************************************************************* */
public:
	/** Returns true if Player Controller is ready to setup all the inputs. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool CanBindInputActions() const;

	/** Adds given contexts to the list of auto managed and binds their input actions . */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetupInputContexts(const TArray<UBmrInputMappingContext*>& InputContexts);
	void SetupInputContexts(const TArray<const UBmrInputMappingContext*>& InputContexts);
	void RemoveInputContexts(const TArray<const UBmrInputMappingContext*>& InputContexts);

	/** Prevents built-in slate input on UMG. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DisplayName = "Set UI Input Ignored"))
	void SetUIInputIgnored();

	/** Takes all cached inputs contexts and turns them on or off according given game state.
	 * @param bEnable If true, all matching contexts will be enabled. If false, all matching contexts will be disabled.
	 * @param CurrentGameState Game state to check matching.
	 * @param bInvertRest If true, all other not matching contexts will be toggled to the opposite of given state (!bEnable).
	 * @see ABmrPlayerController::AddInputContexts */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetAllInputContextsEnabled(bool bEnable, EBmrCurrentGameState CurrentGameState, bool bInvertRest = false);

	/** Enables all managed input contexts by current game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplyAllInputContexts();

	/** Enables or disables specified input context.
	 * @param bEnable If true, the context will be enabled. If false, the context will be disabled.
	 * @param InInputContext The input context to enable or disable. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetInputContextEnabled(bool bEnable, const UBmrInputMappingContext* InInputContext);

	/** Set up input bindings in given contexts. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void BindInputActionsInContext(const UBmrInputMappingContext* InInputContext);

	/** Adds input contexts to the list to be auto turned of or on according current game state.
	 * Make sure UBmrInputMappingContext::ActiveForStates is set.
	 * @param InputContexts Contexts to manage.
	 * @see ABmrPlayerController::AllInputContexts */
	void AddNewInputContexts(const TArray<const UBmrInputMappingContext*>& InputContexts);

	/** Returns the component that responsible for mouse-related logic like showing and hiding itself. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UBmrMouseActivityComponent* GetMouseActivityComponent() const { return MouseComponent; }

	UBmrMouseActivityComponent& GetMouseActivityComponentChecked() const;

	/*********************************************************************************************
	 * Camera
	 ********************************************************************************************* */
public:
	/** Is overriden to setup camera manager once spawned. */
	virtual void SpawnPlayerCameraManager() override;

	/** Is overriden to return correct camera location and rotation for the player. */
	virtual void GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const override;

	/** Is called by ToggleDebugCamera cheat (in build) and F8 button (in editor).
	 * @param bEnable true, when player moves the camera around the level freely during the game in editor or build.
	 * Is not wrapped by WITH_EDITOR as might be called in the build. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (DevelopmentOnly))
	bool bIsDebugCameraEnabled = false;

#if WITH_EDITOR
	/** Is called in editor by F8 button, when switched between PIE and SIE during the game to handle the Debug Camera. */
	void OnPreSwitchBeginPIEAndSIE(bool bIsPIE);
#endif // WITH_EDITOR
};