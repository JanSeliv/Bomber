// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/MySkeletalMeshComponent.h"
//---
#include "NewMainMenuSpotComponent.generated.h"

/**
 * Represents a spot where a character can be selected in the Main Menu.
 * Is added dynamically to the My Skeletal Mesh actors on the level.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWMAINMENU_API UNewMainMenuSpotComponent final : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public function
	 ********************************************************************************************* */
public:
	/** Returns the data asset that contains all the assets and tweaks of New Main Menu game feature.
	 * @see UNewMainMenuSpotComponent::NewMainMenuDataAssetInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE class UNewMainMenuDataAsset* GetNewMainMenuDataAsset() const { return NewMainMenuDataAssetInternal.LoadSynchronous(); }

	/** Guarantees that the data asset is loaded, otherwise, it will crash. */
	const class UNewMainMenuDataAsset& GetNewMainMenuDataAssetChecked() const;

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
	int32 GetSubsequenceTotalFrames(int32 SubsequenceIndex) const;

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains all the assets and tweaks of New Main Menu game feature. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "New Main Menu Data Asset"))
	TSoftObjectPtr<const class UNewMainMenuDataAsset> NewMainMenuDataAssetInternal = nullptr;

	/** Cached cinematic player of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Idle Player"))
	TObjectPtr<class ULevelSequencePlayer> MasterPlayerInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Loads cinematic of this spot.
	 * @see UNewMainMenuSpotComponent::CinematicInternal*/
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void CreateMasterSequencePlayer();

	/** Plays idle in loop of this spot. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void PlayLoopIdle();
};
