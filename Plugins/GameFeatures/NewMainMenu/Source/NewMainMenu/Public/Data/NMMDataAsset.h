// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrManageableWidgetData.h"

#include "NMMDataAsset.generated.h"

enum class ENMMState : uint8;
enum class EBmrLevelType : uint8;

/**
 * Contains common data of the New Main Menu plugin to be tweaked.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNMMDataAsset : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * General
	 ********************************************************************************************* */
public:
	/** Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set. */
	static const UNMMDataAsset& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the data table with the cinematics to be played. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	const FORCEINLINE class UDataTable* GetCinematicsDataTable() const { return CinematicsDataTable; }

	/** Returns data for the Main Menu widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	const FORCEINLINE FBmrManageableWidgetData& GetMainMenuWidgetData() const { return MainMenuWidgetData; }

	/** Returns data for the In Cinematic State widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	const FORCEINLINE FBmrManageableWidgetData& GetInCinematicStateWidgetData() const { return InCinematicStateWidgetData; }

protected:
	/** The data table with the cinematics to be played. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<const class UDataTable> CinematicsDataTable = nullptr;

	/** Data for the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (BlueprintProtected))
	FBmrManageableWidgetData MainMenuWidgetData = FBmrManageableWidgetData::Empty;

	/** Data for the In Cinematic State widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (BlueprintProtected))
	FBmrManageableWidgetData InCinematicStateWidgetData = FBmrManageableWidgetData::Empty;

	/*********************************************************************************************
	 * Camera
	 ********************************************************************************************* */
public:
	/** Returns the duration of transitioning between Main Menu spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE float GetCameraTransitionTime() const { return CameraTransitionTime; }

	/** Returns the duration of blending on start and end Transition state: from Camera Spot to Rail and from Rail to Camera Spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE float GetCameraBlendTime() const { return CameraBlendTime; }

protected:
	/** Duration of transitioning between Main Menu spots.
	 * @warning It has to be greater than 0. To disable camera transition, player has to toggle 'Instant Character Switch' setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (BlueprintProtected, ShowOnlyInnerProperties, ClampMin = "0.01"))
	float CameraTransitionTime = 1.f;

	/** Duration of blending on start and end Transition state: from Camera Spot to Rail and from Rail to Camera Spot.
	 * @warning it is used only when player has enabled 'Instant Character Switch' setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (BlueprintProtected, ShowOnlyInnerProperties, ClampMin = "0.0"))
	float CameraBlendTime = 0.25f;

	/*********************************************************************************************
	 * Input
	 ********************************************************************************************* */
public:
	/** Returns first input context by given game state.
	 * @see UNMMDataAsset::InputContexts.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	const class UBmrInputMappingContext* GetInputContext(ENMMState MenuState) const;

	/** Returns all input contexts.
	 * @see UNMMDataAsset::InputContexts.*/
	void GetAllInputContexts(TArray<const class UBmrInputMappingContext*>& OutInputContexts) const;

	/** Returns the time to hold the skip cinematic button to skip the cinematic. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE float GetSkipCinematicHoldTime() const { return SkipCinematicHoldTime; }

protected:
	/** List of input contexts to manage according their Main Menu States. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TMap<ENMMState, TObjectPtr<const class UBmrInputMappingContext>> InputContexts;

	/** The time to hold the skip cinematic button to skip the cinematic. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	float SkipCinematicHoldTime = 1.f;

	/*********************************************************************************************
	 * Sounds
	 ********************************************************************************************* */
public:
	/** Returns the sound of cinematics music. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE class USoundClass* GetCinematicsSoundClass() const { return CinematicsSoundClass; }

	/** Returns the main menu music of specified level.
	 * @see UBmrSoundsDataAsset::MainMenuMusic */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	class USoundBase* GetMainMenuMusic(EBmrLevelType LevelType) const;

	/** Returns all main menu music.
	 * @see UBmrSoundsDataAsset::MainMenuMusic */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	void GetAllMainMenuMusic(TArray<class USoundBase*>& OutMainMenuMusic) const;

protected:
	/** The sound of cinematics music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> CinematicsSoundClass = nullptr;

	/** Contains all sounds of each level in the main menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TMap<EBmrLevelType, TObjectPtr<class USoundBase>> MainMenuMusic;
};
