// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "NMMSpotsSubsystem.generated.h"

class UNMMSpotComponent;

enum class EBmrLevelType : uint8;
enum class ENMMState : uint8;
enum class EBmrCurrentGameState : uint8;

/**
 * Manages Main Menu cinematic spots and keeps their data.
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMSpotsSubsystem : public UWorldSubsystem
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

	/** Returns the index of the currently selected Main-Menu spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE int32 GetActiveMenuSpotIndex() const { return ActiveMenuSpotIdx; }

	/** Returns an incrementer of the last Main-Menu spot direction, is used to determine the direction of the last move. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE int32 GetLastMoveSpotDirection() const { return LastMoveSpotDirection; }

	/** Add new Main-Menu spot, so it can be obtained by other objects. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void AddNewMainMenuSpot(UNMMSpotComponent* NewMainMenuSpotComponent);

	/** Removes Main-Menu spot if should not be available by other objects anymore. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void RemoveMainMenuSpot(UNMMSpotComponent* MainMenuSpotComponent);

	/** Returns currently selected Main-Menu spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	UNMMSpotComponent* GetCurrentSpot() const;

	/** Returns Main-Menu spots by given level type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	void GetMainMenuSpotsByLevelType(TArray<UNMMSpotComponent*>& OutSpots, EBmrLevelType LevelType) const;

	/** Returns next or previous Main-Menu spot by given incrementer.
	 * It never exits the bounds of the array by going to the last or first element.
	 * @param Incrementer 1 to move right, -1 to move left.
	 * @param LevelType Level type to search in.
	 * @return New active Main-Menu spot component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	UNMMSpotComponent* GetNextSpot(int32 Incrementer, EBmrLevelType LevelType) const;

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
	/** Index of the currently selected Main-Menu spot, is according row index in Cinematics table.
	 * @see FNMMCinematicRow::RowIndex. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	int32 ActiveMenuSpotIdx = 0;

	/** Incrementer of the last Main-Menu spot direction, is used to determine the direction of the last move. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	int32 LastMoveSpotDirection = 0;

	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	TArray<TObjectPtr<UNMMSpotComponent>> MainMenuSpots;

	/** Attempts to switch the active menu spot if current slot is not available for any reason. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void HandleUnavailableMenuSpot();

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
	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(ENMMState NewState, ENMMState PreviousState);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(EBmrCurrentGameState CurrentGameState);
};
