// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "NewMainMenuDataAsset.generated.h"

/**
 * Contains common data of the New Main Menu plugin to be tweaked.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNewMainMenuDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set. */
	static const UNewMainMenuDataAsset& Get();

	/** Returns the data table with the cinematics to be played.
	 * @see UNewMainMenuDataAsset::CinematicsDataTableInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class UDataTable* GetCinematicsDataTable() const { return CinematicsDataTableInternal; }

	/** Returns a class of the Main Menu widget.
	 * @see UNewMainMenuDataAsset::MainMenuWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UNewMainMenuWidget> GetMainMenuWidgetClass() const { return MainMenuWidgetClassInternal; }

	/** Returns a class of the In Cinematic State widget.
	 * @see UNewMainMenuDataAsset::InCinematicStateWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UInCinematicStateWidget> GetInCinematicStateWidgetClass() const { return InCinematicStateWidgetClassInternal; }

protected:
	/** The data table with the cinematics to be played. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Cinematics Data Table", ShowOnlyInnerProperties))
	TObjectPtr<const class UDataTable> CinematicsDataTableInternal = nullptr;

	/** The class of the Main Menu widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main Menu Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UNewMainMenuWidget> MainMenuWidgetClassInternal = nullptr;

	/** The class of the In Cinematic State widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In Cinematic State Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UInCinematicStateWidget> InCinematicStateWidgetClassInternal = nullptr;
};
