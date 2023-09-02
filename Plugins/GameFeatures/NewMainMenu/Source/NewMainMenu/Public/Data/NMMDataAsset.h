// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "NMMDataAsset.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Contains common data of the New Main Menu plugin to be tweaked.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNMMDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set. */
	static const UNMMDataAsset& Get();

	/** Returns the data table with the cinematics to be played.
	 * @see UNMMDataAsset::CinematicsDataTableInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class UDataTable* GetCinematicsDataTable() const { return CinematicsDataTableInternal; }

	/** Returns a class of the Main Menu widget.
	 * @see UNMMDataAsset::MainMenuWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UNewMainMenuWidget> GetMainMenuWidgetClass() const { return MainMenuWidgetClassInternal; }

	/** Returns a class of the In Cinematic State widget.
	 * @see UNMMDataAsset::InCinematicStateWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UNMMCinematicStateWidget> GetInCinematicStateWidgetClass() const { return InCinematicStateWidgetClassInternal; }

	/** Returns first input context by given game state.
	 * @see UNMMDataAsset::InputContextsInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class UMyInputMappingContext* GetInputContext(ECurrentGameState CurrentGameState) const;

	/** Returns all input contexts.
	 * @see UNMMDataAsset::InputContextsInternal.*/
	void GetAllInputContexts(TArray<const class UMyInputMappingContext*>& OutInputContexts) const;

	/** Returns the sound of cinematics music.
	 * @see UNMMDataAsset::CinematicsSoundClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundClass* GetCinematicsSoundClass() const { return CinematicsSoundClassInternal; }

protected:
	/** The data table with the cinematics to be played. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Cinematics Data Table", ShowOnlyInnerProperties))
	TObjectPtr<const class UDataTable> CinematicsDataTableInternal = nullptr;

	/** The class of the Main Menu widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main Menu Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UNewMainMenuWidget> MainMenuWidgetClassInternal = nullptr;

	/** The class of the In Cinematic State widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In Cinematic State Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UNMMCinematicStateWidget> InCinematicStateWidgetClassInternal = nullptr;

	/** List of input contexts to manage according their game states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Input Contexts"))
	TArray<TObjectPtr<const class UMyInputMappingContext>> InputContextsInternal;

	/** The sound of cinematics music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Music Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> CinematicsSoundClassInternal = nullptr;
};
