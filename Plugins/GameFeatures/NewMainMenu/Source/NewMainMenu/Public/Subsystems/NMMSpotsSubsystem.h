// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/ModularGameFeaturePluginSubsystem.h"

#include "NMMSpotsSubsystem.generated.h"

class UNMMSpotComponent;

/**
 * Manages Main Menu cinematic spots and keeps their data.
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMSpotsSubsystem : public UModularGameFeaturePluginSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMSpotsSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNMMOnSpotReady, UNMMSpotComponent*, MainMenuSpotComponent);

	/** Called when the spot is fully initialized: is spawned on the level and finished loading its Master Sequence. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[NewMainMenu]")
	FNMMOnSpotReady OnActiveMenuSpotReady;

	/** Returns true if any Main-Menu spot is fully initialized: spawned on the level and finished loading its Master Sequence. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	bool IsActiveMenuSpotReady() const;

	/** Returns the priority of the currently selected Main-Menu spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE int32 GetActiveSpotPriority() const { return ActiveSpotPriority; }

	/** Sets the active spot priority, allowing external components to select a spot by its priority value. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetActiveSpotPriority(int32 NewPriority) { ActiveSpotPriority = NewPriority; }

	/** Returns an incrementer of the last Main-Menu spot direction, is used to determine the direction of the last move. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE int32 GetLastMoveSpotDirection() const { return LastMoveSpotDirection; }

	/** Add new Main-Menu spot, so it can be obtained by other objects. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void AddNewMainMenuSpot(UNMMSpotComponent* NewMainMenuSpotComponent);

	/** Reinitializes cinematic data for all spots from Data Registry. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void ReinitializeAllSpots();

	/** Called by a spot when its Master Sequence finished async loading, evaluates active spot once all spots are ready. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void NotifySpotLoaded(UNMMSpotComponent* SpotComponent);

	/** Returns true if all spots with cinematic data have finished loading their Master Sequences. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	bool AreAllSpotsLoaded() const;

	/** Removes Main-Menu spot if should not be available by other objects anymore. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void RemoveMainMenuSpot(UNMMSpotComponent* MainMenuSpotComponent);

	/** Returns currently selected Main-Menu spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	UNMMSpotComponent* GetCurrentSpot() const;

	/** Returns all valid Main-Menu spots sorted by priority */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	void GetMainMenuSpots(TArray<UNMMSpotComponent*>& OutSpots) const;

	/** Returns next or previous Main-Menu spot by given incrementer.
	 * It never exits the bounds of the array by going to the last or first element.
	 * @param Incrementer 1 to move right, -1 to move left.
	 * @return New active Main-Menu spot component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	UNMMSpotComponent* GetNextSpot(int32 Incrementer) const;

	/** Goes to another Spot to show another player character on current level.
	 * @param Incrementer 1 to move right, -1 to move left.
	 * @return New active Main-Menu spot component. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	UNMMSpotComponent* MoveMainMenuSpot(int32 Incrementer);

	/** Is code alternative to MoveMainMenuSpot, but allows to use custom predicate,
	 * where incrementer would behave as a step to the next spot, meaning it might be performed several times.
	 * E.g: -1 will move left until the predicate is true, so it might skip few previous spots if they don't match the predicate. */
	UNMMSpotComponent* MoveMainMenuSpotByPredicate(int32 Incrementer, const TFunctionRef<bool(UNMMSpotComponent*)>& Predicate);

protected:
	/** Priority of the currently selected Main-Menu spot, matches FBmrCinematicRow::Priority. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	int32 ActiveSpotPriority = INDEX_NONE;

	/** Incrementer of the last Main-Menu spot direction, is used to determine the direction of the last move. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	int32 LastMoveSpotDirection = 0;

	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	TArray<TObjectPtr<UNMMSpotComponent>> MainMenuSpots;

	/** Selects the highest-priority loaded spot as active and broadcasts OnActiveMenuSpotReady. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void TryBroadcastOnActiveMenuSpotReady();

	/** Attempts to switch the active menu spot if current slot is not available for any reason. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void HandleUnavailableMenuSpot();

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Subscribes to menu state and game state events */
	virtual void OnGameFeatureInitialize_Implementation() override;

	/** Clears all transient data contained in this subsystem */
	virtual void OnGameFeatureDeinitialize_Implementation() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(const struct FGameplayEventData& Payload);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when cinematic Data Registry rows change and all their soft references finish async loading */
	UFUNCTION(BlueprintNativeEvent, Category = "[NewMainMenu]")
	void OnCinematicRowsChanged();
};
