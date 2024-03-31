// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "Data/NMMTypes.h" // ENMMState
//---
#include "NMMBaseSubsystem.generated.h"

class UNMMDataAsset;

enum class ECurrentGameState : uint8;

/**
 * Provides access to the most important data like Data Asset and current state.
 */
UCLASS(BlueprintType, Blueprintable, Config = "NewMainMenu", DefaultConfig)
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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNMMOnStateChanged, ENMMState, NewState);

	/** Called when the state of New Main Menu game feature was changed.
	 * Is local and not replicated. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FNMMOnStateChanged OnMainMenuStateChanged;

	/** Applies the new state of New Main Menu game feature.
	 * Is local and not replicated. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetNewMainMenuState(ENMMState NewState);

	/** Returns the current state of New Main Menu game feature. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE ENMMState GetCurrentMenuState() const { return CurrentMenuStateInternal; }

protected:
	/** Contains the current state of New Main Menu game feature.
	 * Is local and not replicated. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current State"))
	ENMMState CurrentMenuStateInternal = ENMMState::None;

	/*********************************************************************************************
	 * Data Asset
	 ********************************************************************************************* */
public:
	/** Returns the data asset that contains all the assets and tweaks of New Main Menu game feature.
	 * @see UNMMBaseSubsystem::NewMainMenuDataAssetInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const UNMMDataAsset* GetNewMainMenuDataAsset() const;

protected:
	/** Contains all the assets and tweaks of New Main Menu game feature.
	 * Note: Since Subsystem is code-only, is is config property set in NewMainMenu.ini.
	 * Property is put to subsystem because its instance is created before any other object.
	 * It can't be put to DevelopSettings class because it does work properly for MGF-modules. */
	UPROPERTY(Config, VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "New Main Menu Data Asset"))
	TSoftObjectPtr<const UNMMDataAsset> NewMainMenuDataAssetInternal;

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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
