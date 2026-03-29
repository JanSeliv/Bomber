// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// NMM
#include "Data/NMMTypes.h" // ENMMState

#include "NMMBaseSubsystem.generated.h"

class UNMMDataAsset;

/**
 * Provides access to the most important data like Data Asset and current state.
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMBaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMBaseSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * New Main Menu State
	 * Is local for each player and not replicated.
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNMMOnStateChanged, ENMMState, NewState, ENMMState, PreviousState);

	/** Called when the state of New Main Menu game feature was changed.
	 * Is local and not replicated. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[NewMainMenu]")
	FNMMOnStateChanged OnMainMenuStateChanged;

	/** Applies the new state of New Main Menu game feature.
	 * Is local and not replicated. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetNewMainMenuState(ENMMState NewState);

	/** Returns the current state of New Main Menu game feature. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE ENMMState GetCurrentMenuState() const { return CurrentMenuState; }

	/** Returns true if DR_Cinematics Data Registry has any cached rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	static bool HasCinematicRows();

protected:
	/** Contains the current state of New Main Menu game feature.
	 * Is local and not replicated. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	ENMMState CurrentMenuState = ENMMState::None;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called when the world is initialized. */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Clears all transient data contained in this subsystem. */
	virtual void Deinitialize() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the current game state was changed, handles Main Menu states accordingly. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when DR_Cinematics Data Registry cache version changes (rows injected or removed). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnCinematicsRegistryChanged(class UDataRegistry* CinematicsDataRegistry);

	/** Called when a cinematic spot finished loading, re-evaluates whether to transition from BasicMenu to Idle. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnActiveMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);
};

/** Helper macro to bind and call the function when the game state was changed. */
#define BIND_ON_MENU_STATE_CHANGED(Obj, Function)                                \
	{                                                                            \
		UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();             \
		BaseSubsystem.OnMainMenuStateChanged.AddUniqueDynamic(Obj, &Function);   \
		if (BaseSubsystem.GetCurrentMenuState() != ENMMState::None)              \
		{                                                                        \
			Obj->Function(BaseSubsystem.GetCurrentMenuState(), ENMMState::None); \
		}                                                                        \
	}