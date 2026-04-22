// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/BmrSkeletalMeshComponent.h"

// NMM
#include "Data/NmmStateTag.h"
#include "DataRegistries/BmrCinematicRow.h"

#include "NMMSpotComponent.generated.h"

class ULevelSequence;

enum class ENMMCameraRailTransitionState : uint8;

/**
 * Represents a spot where a character can be selected in the Main Menu.
 * Is responsible for:
 * - playing cinematics (animation) in the Menu
 * - Applying the player mesh to the in-game character.
 * Is added dynamically to the Bmr Skeletal Mesh actors on the level.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNMMSpotComponent final : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public function
	 ********************************************************************************************* */
public:
	/** Default constructor. */
	UNMMSpotComponent();

	/** Returns true if this spot is currently selected and possessed by player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	bool IsCurrentSpot() const;

	/** Returns true if this spot is visible, unlocked and can be selected by player.
	 * To make this spot unavailable, call SetActive(false) on this spot ot its skeletal mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	bool IsSpotAvailable() const;

	/** Returns true if this spot current skin is unlocked and can be selected by player.
	 * To make this spot skin unavailable, call SetSkinAvailable(false, skinIndex) on this spot on its skeletal mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	bool IsSpotSkinAvailable() const;

	/** Returns the Skeletal Mesh of the Bomber character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	class UBmrSkeletalMeshComponent* GetMeshComponent() const;
	class UBmrSkeletalMeshComponent& GetMeshChecked() const;

	/** Returns the owner of this component as Bomber Skeletal Mesh actor. */
	class ABmrSkeletalMeshActor& GetOwnerChecked() const;

	/** Sets the look of this spot to the in-game player character. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void ApplyMeshOnPlayer();

	/*********************************************************************************************
	 * Cinematics
	 ********************************************************************************************* */
public:
	/** Returns cinematic row of this spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	const FBmrCinematicRow& GetCinematicRow() const { return CinematicRow; }

	/** Returns cached cinematic player of this spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE class ULevelSequencePlayer* GetMasterPlayer() const { return MasterPlayer; }

	/** Returns main cinematic of this spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	ULevelSequence* GetMasterSequence() const;

	/** Reinitializes cinematic data from Data Registry: cleans up current cinematic and re-queries for a matching row. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void ReinitializeCinematicData();

	/** Prevents the spot from playing any cinematic. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void StopMasterSequence();

	/** Returns true if current game state can be eventually changed. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	bool CanChangeCinematicState(FNmmStateTag NewMenuState) const;

	/** Activate given cinematic state on this spot. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetCinematicByState(FNmmStateTag MenuState);

	/** Creates MasterPlayer from a loaded cinematic asset and notifies the Spots Subsystem, no-op if not yet loaded or already initialized. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void InitMasterSequencePlayer();

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Cached cinematic player of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	TObjectPtr<class ULevelSequencePlayer> MasterPlayer = nullptr;

	/** Cached Cinematic Row that contains data about this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	FBmrCinematicRow CinematicRow = FBmrCinematicRow::Empty;

	/** Current cinematic state of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	FNmmStateTag CinematicState = FNmmStateTag::None;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Clears all transient data created by this component. */
	virtual void OnUnregister() override;

	/** Obtains and caches cinematic data from the table to this spot.
	 * @see UNMMSpotComponent::CinematicRow */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void UpdateCinematicData();

	/** Marks own cinematic as seen by player for the save system. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void MarkCinematicAsSeen();

	/** Triggers or stops cinematic by current state. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void ApplyCinematicState();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the NMM data asset is loaded and available. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnDataAssetLoaded(const class UNMMDataAsset* DataAsset);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(const struct FGameplayEventData& Payload);

	/** Called when the sequence is paused or when cinematic was ended. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnMasterSequencePaused();

	/** Called when the Camera Rail transition state changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnCameraRailTransitionStateChanged(ENMMCameraRailTransitionState CameraRailTransitionStateChanged);
};
