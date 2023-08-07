// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/MySkeletalMeshComponent.h"
//---
#include "Data/NMMTypes.h"
//---
#include "NMMSpotComponent.generated.h"

enum class ECurrentGameState : uint8;

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
	class ULevelSequence* GetMasterSequence() const;

	/** Finds subsequence of this spot by given index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const ULevelSequence* FindSubsequence(int32 SubsequenceIndex) const;

	/** Returns the length of by given subsequence index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetSequenceTotalFrames(const ULevelSequence* LevelSequence);

	/** Prevents the spot from playing any cinematic. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopMasterSequence();

	/** Plays idle part in loop of current Master Sequence. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayIdlePart();

	/** Plays main part of current Master Sequence. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayMainPart();

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
	void CreateMasterSequencePlayer();

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
