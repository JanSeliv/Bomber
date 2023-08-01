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

	/** Returns the Skeletal Mesh of the Bomber character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const;
	class UMySkeletalMeshComponent& GetMeshChecked() const;

	/** Returns main cinematic of this spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class ULevelSequence* GetMasterSequence() const { return MasterSequenceInternal; }

	/** Blends camera to this spot. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCameraViewOnSpot(bool bBlend);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains all the assets and tweaks of New Main Menu game feature. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "New Main Menu Data Asset"))
	TSoftObjectPtr<const class UNewMainMenuDataAsset> NewMainMenuDataAssetInternal = nullptr;

	/** Linked camera actor to set the view that is also used by cinematics. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Camera Actor"))
	TObjectPtr<class ACameraActor> CameraActorInternal = nullptr;

	/** Cached Master Sequence of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Master Sequence"))
	TObjectPtr<class ULevelSequence> MasterSequenceInternal = nullptr;

	/** Cached cinematic of this spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Idle Player"))
	TObjectPtr<class ULevelSequencePlayer> IdlePlayerInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Sets camera view to this spot if current level type is equal to the spot's player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TrySetCameraViewByDefault();

	/** Loads cinematic of this spot.
	 * @see UNewMainMenuSpotComponent::CinematicInternal*/
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void LoadMasterSequence();
	void OnMasterSequenceLoaded(TSoftObjectPtr<ULevelSequence> LoadedMasterSequence);

	/** Plays idle in loop of this spot.
	 * @see UNewMainMenuSpotComponent::CinematicInternal*/
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void PlayLoopIdle();
};
