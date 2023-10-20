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
UCLASS(BlueprintType, Blueprintable, Config = "NewMainMenu", DefaultConfig)
class NEWMAINMENU_API UNMMSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMainMenuSpotReady, UNMMSpotComponent*, MainMenuSpotComponent);

	/** Called when the spot was spawned on new level and it finished loading its Master Sequence. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnMainMenuSpotReady OnMainMenuSpotReady;

	/** Returns the data asset that contains all the assets and tweaks of New Main Menu game feature.
	 * @see UNMMSubsystem::NewMainMenuDataAssetInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class UNMMDataAsset* GetNewMainMenuDataAsset() const;

	/** Add new Main-Menu spot, so it can be obtained by other objects. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddNewMainMenuSpot(UNMMSpotComponent* NewMainMenuSpotComponent);

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
	/** Contains all the assets and tweaks of New Main Menu game feature.
	 * Note: Since Subsystem is code-only, is is config property set in NewMainMenu.ini.
	 * Property is put to subsystem because its instance is created before any other object.
	 * It can't be put to DevelopSettings class because it does work properly for MGF-modules. */
	UPROPERTY(Config, VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "New Main Menu Data Asset"))
	TSoftObjectPtr<const class UNMMDataAsset> NewMainMenuDataAssetInternal;

	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main-Menu Spots"))
	TArray<TObjectPtr<UNMMSpotComponent>> MainMenuSpotsInternal;

	/** Index of the currently selected Main-Menu spot, is according row index in Cinematics table.
	 * @see FCinematicRow::RowIndex. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Active Main-Menu Spot Index"))
	int32 ActiveMainMenuSpotIdx = 0;
};
