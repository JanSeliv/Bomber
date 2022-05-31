// Copyright (c) Yevhenii Selivanov.

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
	void GetAllInputContexts(TArray<const class UMyInputMappingContext*>& OutInputContexts) const;

	/** Returns the overall amount of all gameplay input contexts. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	int32 GetGameplayInputContextsNum() const { return GameplayInputContextClassesInternal.Num(); }

	/** Returns the Enhanced Input Mapping Context of gameplay actions for specified local player.
	* @param LocalPlayerIndex The index of a local player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class UMyInputMappingContext* GetGameplayInputContext(int32 LocalPlayerIndex) const;

	/** Returns the Enhanced Input Mapping Context of actions on the Main Menu widget.
	* @see UPlayerInputDataAsset::MainMenuInputContextInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE class UMyInputMappingContext* GetMainMenuInputContext() const { return MainMenuInputContextInternal; }

	/** Returns the Enhanced Input Mapping Context of actions on the In-Game Menu widget.
	  * @see UPlayerInputDataAsset:: */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE class UMyInputMappingContext* GetInGameMenuInputContext() const { return InGameMenuInputContextInternal; }

	/** Returns the Enhanced Input Mapping Context of actions on the Settings widget.
	  * @see ::SettingsInputContextInternalInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE class UMyInputMappingContext* GetSettingsInputContext() const { return SettingsInputContextInternalInternal; }

	/** Returns true if specified key is mapped to any gameplay input context. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Key"))
	bool IsMappedKey(const FKey& Key) const;

protected:
	/** Enhanced Input Mapping Contexts of gameplay actions for local players
	 *  Are selectable classes instead of objects directly to avoid changing data asset by MapKey\UnmapKey. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Gameplay Input Context Classes", ShowOnlyInnerProperties))
	TArray<TSubclassOf<UMyInputMappingContext>> GameplayInputContextClassesInternal; //[D]

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main Menu Input Context", ShowOnlyInnerProperties))
	TObjectPtr<class UMyInputMappingContext> MainMenuInputContextInternal = nullptr; //[D]

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In-Game Menu Input Context", ShowOnlyInnerProperties))
	TObjectPtr<class UMyInputMappingContext> InGameMenuInputContextInternal = nullptr; //[D]

	/** Enhanced Input Mapping Context of actions on the Settings widget*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Input Context", ShowOnlyInnerProperties))
	TObjectPtr<class UMyInputMappingContext> SettingsInputContextInternalInternal = nullptr; //[D]

	/** Creates new contexts if is needed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryCreateGameplayInputContexts() const;

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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	bool CanHideMouse() const;

	/** Called to to set the mouse cursor visibility.
	 * @param bShouldShow true to show mouse cursor, otherwise hide it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseVisibility(bool bShouldShow);

	/** If true, set the mouse focus on game and UI, otherwise only focusing on game inputs. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseFocusOnUI(bool bFocusOnUI);

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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	bool IsInputContextEnabled(const UMyInputMappingContext* InputContext) const;

	/** Enables or disables specified input context.
	 * @param bEnable set true to add specified input context, otherwise it will be removed from local player.
	 * @param InputContext Context to set. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InputContext);

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
