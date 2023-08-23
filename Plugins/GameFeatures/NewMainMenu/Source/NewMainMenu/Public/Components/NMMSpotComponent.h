// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/MySkeletalMeshComponent.h"
//---
#include "Data/NMMTypes.h"
//---
#include "NMMSpotComponent.generated.h"

enum class ECurrentGameState : uint8;

class ULevelSequence;

/**
 * Represents a spot where a character can be selected in the Main Menu.
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

	/** Returns main cinematic of this spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ULevelSequence* GetMasterSequence() const;

	/** Prevents the spot from playing any cinematic. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopMasterSequence();

	/** Returns in which state the cinematic is currently playing
	 * @see CinematicStateInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE ENMMCinematicState GetCurrentCinematicState() const { return CinematicStateInternal; }

	/** Activate given cinematic state on this spot. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCinematicState(ENMMCinematicState CinematicState);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Cached cinematic player of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Idle Player"))
	TObjectPtr<class ULevelSequencePlayer> MasterPlayerInternal = nullptr;

	/** Cached Cinematic Row that contains data about this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Cinematic Row"))
	FNMMCinematicRow CinematicRowInternal = FNMMCinematicRow::Empty;

	/** Current cinematic state of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Cinematic State"))
	ENMMCinematicState CinematicStateInternal = ENMMCinematicState::None;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Is called to start listening game state changes. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnGameStateChanged(class AMyGameStateBase* MyGameState);

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

	/** Starts viewing through camera of current cinematic or gameplay one depending on given state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void PossessCamera(ENMMCinematicState CinematicState);

	/** Marks own cinematic as seen by player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryMarkCinematicAsSeen();
	
	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when the sequence is paused or when cinematic was ended. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMasterSequencePaused();
};
