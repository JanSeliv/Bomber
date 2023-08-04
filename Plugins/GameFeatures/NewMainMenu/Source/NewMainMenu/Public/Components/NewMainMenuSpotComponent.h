// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/MySkeletalMeshComponent.h"
//---
#include "NewMainMenuSpotComponent.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Represents a spot where a character can be selected in the Main Menu.
 * Is added dynamically to the My Skeletal Mesh actors on the level.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNewMainMenuSpotComponent final : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public function
	 ********************************************************************************************* */
public:
	/** Default constructor. */
	UNewMainMenuSpotComponent();

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
	/** Returns main cinematic of this spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const class ULevelSequence* GetMasterSequence() const;

	/** Finds subsequence of this spot by given index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const ULevelSequence* FindSubsequence(int32 SubsequenceIndex) const;

	/** Returns the length of by given subsequence index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetSequenceTotalFrames(const ULevelSequence* LevelSequence);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Cached cinematic player of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Idle Player"))
	TObjectPtr<class ULevelSequencePlayer> MasterPlayerInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Loads cinematic of this spot.
	 * @see UNewMainMenuSpotComponent::CinematicInternal*/
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void CreateMasterSequencePlayer();

	/** Plays idle part in loop of current Master Sequence. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void PlayIdlePart();

	/** Plays main part of current Master Sequence. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void PlayMainPart();
};
