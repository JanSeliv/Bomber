// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// NMM
#include "Data/NmmStateTag.h"

#include "NMMDataAsset.generated.h"

/**
 * Contains common data of the New Main Menu plugin to be tweaked.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNMMDataAsset : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set. */
	static const UNMMDataAsset& Get(const UObject* OptionalWorldContext = nullptr);

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
	/** Returns all input contexts matching given menu state.
	 * @see UNMMDataAsset::InputContexts.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	void GetInputContexts(FNmmStateTag MenuState, TArray<class UBmrInputMappingContext*>& OutInputContexts) const;

	/** Returns all input contexts.
	 * @see UNMMDataAsset::InputContexts.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	void GetAllInputContexts(TArray<class UBmrInputMappingContext*>& OutInputContexts) const;

	/** Returns the time to hold the skip cinematic button to skip the cinematic. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE float GetSkipCinematicHoldTime() const { return SkipCinematicHoldTime; }

protected:
	/** List of input contexts to manage according their Main Menu States. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "NMM.State"))
	TMap<TObjectPtr<class UBmrInputMappingContext>, FGameplayTagContainer> InputContexts;

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

	/** Returns the main menu background music */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE class USoundBase* GetMainMenuMusic() const { return MainMenuMusic; }

protected:
	/** The sound of cinematics music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> CinematicsSoundClass = nullptr;

	/** The background music played in the main menu */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<class USoundBase> MainMenuMusic = nullptr;
};
