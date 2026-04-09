// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/ModularGameFeaturePluginSubsystem.h"

// NMM
#include "Data/NmmStateTag.h"

#include "NMMBaseSubsystem.generated.h"

class UNMMDataAsset;

/**
 * Provides access to the most important data like Data Asset and current state.
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMBaseSubsystem : public UModularGameFeaturePluginSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMBaseSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * New Main Menu State
	 * Is local for each player and not replicated.
	 * State changes are broadcast via NmmGameplayTags::Event::MenuStateChanged global message,
	 * where InstigatorTags contains the new FNmmStateTag.
	 ********************************************************************************************* */
public:
	/** Applies the new state of New Main Menu game feature.
	 * Is local and not replicated.
	 * Broadcasts NmmGameplayTags::Event::MenuStateChanged global message. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetNewMainMenuState(FNmmStateTag NewState);

	/** Returns the current state of New Main Menu game feature. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE FNmmStateTag GetCurrentMenuState() const { return CurrentMenuStateTag; }

protected:
	/** Contains the current state of New Main Menu game feature.
	 * Is local and not replicated. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	FNmmStateTag CurrentMenuStateTag = FNmmStateTag::None;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Subscribes to game state events */
	virtual void OnGameFeatureInitialize_Implementation() override;

	/** Clears all transient data contained in this subsystem */
	virtual void OnGameFeatureDeinitialize_Implementation() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the current game state was changed, handles Main Menu states accordingly. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when a cinematic spot finished loading, re-evaluates whether to transition from BasicMenu to Idle. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnActiveMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);
};
