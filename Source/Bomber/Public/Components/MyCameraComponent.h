// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Camera/CameraComponent.h"
#include "Bomber.h"
//---
#include "MyCameraComponent.generated.h"

/**
 * The main camera viewpoint of the game.
 */
UCLASS(Config = "GameUserSettings", ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMyCameraComponent final : public UCameraComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	UMyCameraComponent();

	/** Set the maximum possible height. Called on construct and game start*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateMaxHeight();

	/**
	 * Set the location between players.
	 * @param DeltaTime Optional parameter, lerp if specified
	 * @return true for successful update
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	bool UpdateLocation(float DeltaTime = 0.f);

	/** Returns true if camera does not follow by players.
	 * @see UMyCameraComponent::bLockCameraOnCenterInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsCameraLockedOnCenter() const { return bIsCameraLockedOnCenterInternal; }

	/** Calls to set following camera by player locations.
	 * @param bInCameraLockedOnCenter true to prevent moving camera. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCameraLockedOnCenter(bool bInCameraLockedOnCenter);

	/** Returns the center camera location between all specified cells. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FVector GetCameraLocationBetweenCells(const TSet<FCell>& Cells) const;

	/** Returns the center camera location between all players and bots. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FVector GetCameraLocationBetweenPlayers() const;

	/** Returns the default camera location between all players and bots.
	 * Is absolute center position, where the camera starts game and returns to it on endgame.
	 * Camera always stays there if IsCameraLockedOnCenter() returns true. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FVector GetCameraLockedLocation() const;

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** If true, it will prevent following camera by player locations. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Camera Locked On Center"))
	bool bIsCameraLockedOnCenterInternal; //[C]

	/** The minimal camera height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Min Height"))
	float MinHeightInternal = 1500.f; //[N]

	/** The maximal camera height. Is set dynamically by UMyCameraComponent::UpdateMaxHeights(). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Max Height"))
	float MaxHeightInternal; //[G]

	/** If UMyCameraComponent::StartLocation is true, then should forced moving to the start position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Force Move To Start"))
	bool bForceStartInternal; //[N]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called every frame. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Listen game states to manage the tick. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Starts viewing through this camera. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void PossessCamera();
};
