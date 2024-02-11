// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "NMMSpotsSubsystem.generated.h"

class UNMMSpotComponent;

enum class ELevelType : uint8;

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

	/** Called when the spot was spawned on new level and it finished loading its Master Sequence. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FNMMOnSpotReady OnMainMenuSpotReady;

	/** Add new Main-Menu spot, so it can be obtained by other objects. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddNewMainMenuSpot(UNMMSpotComponent* NewMainMenuSpotComponent);

	/** Removes Main-Menu spot if should not be available by other objects anymore. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void RemoveMainMenuSpot(UNMMSpotComponent* MainMenuSpotComponent);

	/** Returns currently selected Main-Menu spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	UNMMSpotComponent* GetActiveMainMenuSpotComponent() const;

	/** Returns Main-Menu spots by given level type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetMainMenuSpotsByLevelType(TArray<UNMMSpotComponent*>& OutSpots, ELevelType LevelType) const;

	/** Returns next or previous Main-Menu spot by given incrementer.
	 * It never exits the bounds of the array by going to the last or first element.
	 * @param Incrementer 1 to move right, -1 to move left.
	 * @param LevelType Level type to search in.
	 * @return New active Main-Menu spot component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	UNMMSpotComponent* GetNextMainMenuSpotComponent(int32 Incrementer, ELevelType LevelType) const;

	/** Goes to another Spot to show another player character on current level.
	 * @param Incrementer 1 to move right, -1 to move left.
	 * @return New active Main-Menu spot component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	UNMMSpotComponent* MoveMainMenuSpot(int32 Incrementer);

protected:
	/** Index of the currently selected Main-Menu spot, is according row index in Cinematics table.
	 * @see FCinematicRow::RowIndex. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Active Main-Menu Spot Index"))
	int32 ActiveMainMenuSpotIdx = 0;

	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main-Menu Spots"))
	TArray<TObjectPtr<UNMMSpotComponent>> MainMenuSpotsInternal;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** Clears all transient data contained in this subsystem. */
	virtual void Deinitialize() override;
};
