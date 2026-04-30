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

	/** Broadcasts MenuReady global message if not already broadcast and menu prerequisites are met (spots ready or no cinematic rows).
	 * Is called from multiple places (pawn ready, spots loaded) but only the first successful call takes effect */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void TryBroadcastMenuReady();

	/** Predicts the target menu state based on current cinematic readiness.
	 * Is useful to know the desired menu destination before entering, so systems can prepare accordingly */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FNmmStateTag GetPredictedMenuState() const;

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

	/** Recovers menu state after game feature activations. */
	virtual void OnGameFeatureActivated(const class UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the first player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnFirstPawnReady(const struct FGameplayEventData& Payload);

	/** Called when the current game state was changed, handles Main Menu states accordingly. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when a cinematic spot finished loading, re-evaluates whether to transition from BasicMenu to Idle. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnActiveMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);
};
