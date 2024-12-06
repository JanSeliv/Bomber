// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"
//---
#include "Structures/ManageableWidgetData.h"
//---
#include "NMMDataAsset.generated.h"

enum class ENMMState : uint8;

/**
 * Contains common data of the New Main Menu plugin to be tweaked.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNMMDataAsset : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * General
	 ********************************************************************************************* */
public:
	/** Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set. */
	static const UNMMDataAsset& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the data table with the cinematics to be played.
	 * @see UNMMDataAsset::CinematicsDataTableInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class UDataTable* GetCinematicsDataTable() const { return CinematicsDataTableInternal; }

	/** Returns data for the Main Menu widget.
	 * @see UNMMDataAsset::MainMenuWidgetDataInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FManageableWidgetData& GetMainMenuWidgetData() const { return MainMenuWidgetDataInternal; }

	/** Returns data for the In Cinematic State widget.
	 * @see UNMMDataAsset::InCinematicStateWidgetDataInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FManageableWidgetData& GetInCinematicStateWidgetData() const { return InCinematicStateWidgetDataInternal; }

	/** Returns the sound of cinematics music.
	 * @see UNMMDataAsset::CinematicsSoundClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class USoundClass* GetCinematicsSoundClass() const { return CinematicsSoundClassInternal; }

protected:
	/** The data table with the cinematics to be played. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Cinematics Data Table", ShowOnlyInnerProperties))
	TObjectPtr<const class UDataTable> CinematicsDataTableInternal = nullptr;

	/** Data for the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (BlueprintProtected, DisplayName = "Main Menu Widget"))
	FManageableWidgetData MainMenuWidgetDataInternal;

	/** Data for the In Cinematic State widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (BlueprintProtected, DisplayName = "In Cinematic State Widget"))
	FManageableWidgetData InCinematicStateWidgetDataInternal;

	/** The sound of cinematics music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Music Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> CinematicsSoundClassInternal = nullptr;

	/*********************************************************************************************
	 * Camera
	 ********************************************************************************************* */
public:
	/** Returns the duration of transitioning between Main Menu spots.
	* @see UNMMDataAsset::CameraTransitionTimeInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetCameraTransitionTime() const { return CameraTransitionTimeInternal; }

	/** Returns the duration of blending on start and end Transition state: from Camera Spot to Rail and from Rail to Camera Spot.
	 * @see UNMMDataAsset::CameraBlendTimeInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetCameraBlendTime() const { return CameraBlendTimeInternal; }

protected:
	/** Duration of transitioning between Main Menu spots.
	 * @warning It has to be greater than 0. To disable camera transition, player has to toggle 'Instant Character Switch' setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (BlueprintProtected, DisplayName = "Camera Transition Time", ShowOnlyInnerProperties, ClampMin = "0.01"))
	float CameraTransitionTimeInternal = 1.f;

	/** Duration of blending on start and end Transition state: from Camera Spot to Rail and from Rail to Camera Spot.
	 * @warning it is used only when player has enabled 'Instant Character Switch' setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (BlueprintProtected, DisplayName = "Camera Blend Time", ShowOnlyInnerProperties, ClampMin = "0.0"))
	float CameraBlendTimeInternal = 0.25f;

	/*********************************************************************************************
	 * Input
	 ********************************************************************************************* */
public:
	/** Returns first input context by given game state.
	 * @see UNMMDataAsset::InputContextsInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	const class UMyInputMappingContext* GetInputContext(ENMMState MenuState) const;

	/** Returns all input contexts.
	 * @see UNMMDataAsset::InputContextsInternal.*/
	void GetAllInputContexts(TArray<const class UMyInputMappingContext*>& OutInputContexts) const;

	/** Returns the time to hold the skip cinematic button to skip the cinematic.
	 * @see UNMMDataAsset::SkipCinematicHoldTimeInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetSkipCinematicHoldTime() const { return SkipCinematicHoldTimeInternal; }

protected:
	/** List of input contexts to manage according their Main Menu States. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (BlueprintProtected, DisplayName = "Input Contexts", ShowOnlyInnerProperties))
	TMap<ENMMState, TObjectPtr<const class UMyInputMappingContext>> InputContextsInternal;

	/** The time to hold the skip cinematic button to skip the cinematic. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (BlueprintProtected, DisplayName = "Skip Cinematic Hold Time", ShowOnlyInnerProperties))
	float SkipCinematicHoldTimeInternal = 1.f;
};
