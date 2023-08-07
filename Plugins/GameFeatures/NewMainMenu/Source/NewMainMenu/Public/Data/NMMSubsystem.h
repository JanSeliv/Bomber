// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "NMMSubsystem.generated.h"

enum class ELevelType : uint8;

class UNMMSpotComponent;

/**
 * Provides access to the common runtime data about new Main Menu like current cinematic spots.
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMSubsystem& Get();

	/** Add new Main-Menu spot, so it can be obtained by other objects. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddNewMainMenuSpot(class UNMMSpotComponent* NewMainMenuSpotComponent);

	/** Returns currently selected Main-Menu spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	UNMMSpotComponent* GetActiveMainMenuSpotComponent() const;

	/** Returns Main-Menu spots by given level type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetMainMenuSpotsByLevelType(TArray<UNMMSpotComponent*>& OutSpots, ELevelType LevelType) const;

	/** Goes to another Spot to show another player character on current level.
	 * @param Incrementer 1 to move right, -1 to move left.
	 * @return New active Main-Menu spot component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	UNMMSpotComponent* MoveMainMenuSpot(int32 Incrementer);

protected:
	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main-Menu Spots"))
	TArray<TObjectPtr<class UNMMSpotComponent>> MainMenuSpotsInternal;

	/** Index of the currently selected Main-Menu spot, is according row index in Cinematics table.
	 * @see FCinematicRow::RowIndex. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Active Main-Menu Spot Index"))
	int32 ActiveMainMenuSpotIdx = 0;
};
