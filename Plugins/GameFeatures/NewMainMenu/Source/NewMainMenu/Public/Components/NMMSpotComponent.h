// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/MySkeletalMeshComponent.h"
//---
#include "Data/NMMTypes.h"
//---
#include "NMMSpotComponent.generated.h"

class ULevelSequence;

enum class ENMMState : uint8;

/**
 * Represents a spot where a character can be selected in the Main Menu.
 * Is responsible for playing cinematics (animation).
 * Is added dynamically to the My Skeletal Mesh actors on the level.
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

	/** Returns true if this spot is currently active and possessed by player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsActiveSpot() const;

	/** Returns the Skeletal Mesh of the Bomber character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const;
	class UMySkeletalMeshComponent& GetMeshChecked() const;

	/*********************************************************************************************
	 * Cinematics
	 ********************************************************************************************* */
public:
	/** Returns cinematic row of this spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FNMMCinematicRow& GetCinematicRow() const { return CinematicRowInternal; }

	/** Returns cached cinematic player of this spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class ULevelSequencePlayer* GetMasterPlayer() const { return MasterPlayerInternal; }

	/** Returns main cinematic of this spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ULevelSequence* GetMasterSequence() const;

	/** Prevents the spot from playing any cinematic. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopMasterSequence();

	/** Returns true if current game state can be eventually changed. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool CanChangeCinematicState(ENMMState NewMainMenuState) const;

	/** Activate given cinematic state on this spot. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCinematicByState(ENMMState MainMenuState);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Cached cinematic player of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Idle Player"))
	TObjectPtr<class ULevelSequencePlayer> MasterPlayerInternal = nullptr;

	/** Cached Cinematic Row that contains data about this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Cinematic Row"))
	FNMMCinematicRow CinematicRowInternal = FNMMCinematicRow::Empty;

	/** Current cinematic state of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Cinematic State"))
	ENMMState CinematicStateInternal = ENMMState::None;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Clears all transient data created by this component. */
	virtual void OnUnregister() override;

	/** Obtains and caches cinematic data from the table to this spot.
	 * @see UNMMSpotComponent::CinematicRowInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateCinematicData();

	/** Loads cinematic of this spot.
	 * @see UNMMSpotComponent::CinematicInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void LoadMasterSequencePlayer();

	/** Is called when the cinematic was loaded to finish creation. */
	void OnMasterSequenceLoaded(TSoftObjectPtr<ULevelSequence> LoadedMasterSequence);

	/** Marks own cinematic as seen by player for the save system. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void MarkCinematicAsSeen();

	/** Triggers or stops cinematic by current state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyCinematicState();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(ENMMState NewState);

	/** Called when the sequence is paused or when cinematic was ended. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMasterSequencePaused();
};
