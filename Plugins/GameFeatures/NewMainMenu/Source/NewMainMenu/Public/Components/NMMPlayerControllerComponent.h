// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

#include "NMMPlayerControllerComponent.generated.h"

class ABmrPlayerController;

enum class ENMMState : uint8;

/**
 * Represents the Player Controller in the NewMain Menu module, where the Owner is Player Controller actor.
 * Is responsible for managing Main Menu inputs.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNMMPlayerControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UNMMPlayerControllerComponent();

	/** Returns Player Controller of this component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	ABmrPlayerController* GetPlayerController() const;
	ABmrPlayerController& GetPlayerControllerChecked() const;

	/*********************************************************************************************
	 * Main methods
	 ********************************************************************************************* */
public:
	/** Returns loaded and cached Save Game Data of the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE class UNMMSaveGameData* GetSaveGameData() const { return SaveGameData; }

	/** Assigns existing Save Game Data to this component. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetSaveGameData(class USaveGame* NewSaveGameData);

	/** Enables or disables the input context during Cinematic Main Menu State. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetCinematicInputContextEnabled(bool bEnable);

	/** Enables or disables Cinematic mouse settings from Player Input data asset. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetCinematicMouseVisibilityEnabled(bool bEnabled);

	/** Enables or disables the input context according to new menu state. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetManagedInputContextsEnabled(ENMMState NewState);

	/*********************************************************************************************
	 * Sounds
	 ********************************************************************************************* */
public:
	/** Trigger the background music to be played in the Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void PlayMainMenuMusic();

	/** Stops currently played Main Menu background music. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void StopMainMenuMusic();

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains loaded and cached Save Game Data of the Main Menu. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	TObjectPtr<class UNMMSaveGameData> SaveGameData = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the owning Actor begins play or when the component is created if the Actor has already begun play. */
	virtual void BeginPlay() override;

	/** Clears all transient data created by this component */
	virtual void OnUnregister() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the first player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnFirstPawnReady(const struct FGameplayEventData& Payload);

	/** Listen to react when entered the Menu state. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called wen the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(ENMMState NewState, ENMMState PreviousState);

	/** Is listen to set Menu game state once first spot is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnActiveMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);

	/** Is called from AsyncLoadGameFromSlot once Save Game is loaded, or null if it failed to load. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnAsyncLoadGameFromSlotCompleted(class USaveGame* SaveGame);
};
