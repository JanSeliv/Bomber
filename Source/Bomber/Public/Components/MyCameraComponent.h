// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Camera/CameraComponent.h"
//---
#include "MyCameraComponent.generated.h"

enum class ECurrentGameState : uint8;
enum EAspectRatioAxisConstraint : int;

/**
 * Contains parameters to tweak how calculate the distance from camera to the level during the game
 */
USTRUCT(BlueprintType)
struct FCameraDistanceParams
{
	GENERATED_BODY()

	/** The custom additive angle to affect the fit distance calculation from camera to the level.
	 * If 0, then do not apply any additional angle to fit the view.
	 * @see FCameraDistanceParams::CalculateFitViewAdditiveAngle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float FitViewAdditiveAngle = 10.f;

	/** The minimal distance in UU from camera to the level.
	 * If 0, then limit is not applied.
	 * @see FCameraDistanceParams::LimitToMinDistance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float MinDistance = 1500.f;

	/** If set, returns additional FOV modifier scaled by level size and current screen aspect ratio.
	 * @param InOutFOV In: original to modify, out: original + additive scaled by specified size and current screen aspect ratio.*/
	void CalculateFitViewAdditiveAngle(float& InOutFOV) const;

	/** If set, truncates specified distance to allowed minimal one.
	 * @param InOutCameraDistance In: the distance to cut if needed, out: truncated distance. */
	void LimitToMinDistance(float& InOutCameraDistance) const;

	/** Calculates the distance how far away the camera should be placed to fit the given view for specified FOV.
	 * Is useful to avoid the fisheye effect by moving the camera instead of changing the FOV.
	 * @param ViewSizeUU The width (X) and length (Y) in UU of the view target.
	 * @param CameraFOV The FOV for which distance will be found. */
	static float CalculateDistanceToFitViewToFOV(const FVector2D& ViewSizeUU, float CameraFOV);
};

/**
 * The main camera viewpoint of the game.
 */
UCLASS(Config = "GameUserSettings", DefaultConfig, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMyCameraComponent final : public UCameraComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	UMyCameraComponent();

	/** Returns current FOV of camera manager that is more reliable than own FOV. */
	UFUNCTION(BlueprintPure, Category = "C++")
	float GetCameraManagerFOV() const;

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
	 * @param bInCameraLockedOnCenter true to prevent moving camera.
	 * It does not call SaveConfig() for this config property, call it manually if needed. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCameraLockedOnCenter(bool bInCameraLockedOnCenter);

	/** Returns parameters to tweak about the distance calculation from camera to the level during the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCameraDistanceParams& GetCameraDistanceParams() const { return DistanceParamsInternal; }

	/** Allows to tweak distance calculation from camera to the level during the game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCameraDistanceParams(const FCameraDistanceParams& InCameraDistanceParams);

	/** Calculates how faw away the camera should be placed from specified cells. */
	UFUNCTION(BlueprintPure, Category = "C++")
	float GetCameraDistanceToCells(const TSet<struct FCell>& Cells) const;

	/** Returns the center camera location between all players and bots. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FVector GetCameraLocationBetweenPlayers() const;

	/** Returns the default camera location between all players and bots.
	 * Is absolute center position, where the camera starts game and returns to it on endgame.
	 * Camera always stays there if IsCameraLockedOnCenter() returns true. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FVector GetCameraLockedLocation() const;

	/** Starts viewing through this camera. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PossessCamera(bool bBlendCamera = true);

	/** Disable to prevent automatic possessing on Game Starting state, could be disabled by external systems like to show cinematic etc.
	 * @see UMyCameraComponent::bAutoPossessCameraInternal */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAutoPossessCameraEnabled(bool bInAutoPossessCamera);
	
protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** If true, it will prevent following camera by player locations, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Camera Locked On Center"))
	bool bIsCameraLockedOnCenterInternal;

	/** Contains parameters to tweak the distance calculation from camera to the level during the game.
	 * @see UMyCameraComponent::GetCameraDistanceToCells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Camera Distance Params"))
	FCameraDistanceParams DistanceParamsInternal;

	/** When true, camera will be automatically possessed on Game Starting state. Could be disabled by extern system could be disabled by external systems like to show cinematic etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Auto Possess Camera"))
	bool bAutoPossessCameraInternal = true;

	/** When true, force the moving to the start position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Force Move To Start"))
	bool bForceStartInternal = false;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called every frame. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Activates the SceneComponent, should be overridden by native child classes. */
	virtual void Activate(bool bReset) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Listen game states to manage the tick. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Listen to recalculate camera location when screen aspect ratio was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAspectRatioChanged(float NewAspectRatio, EAspectRatioAxisConstraint NewAxisConstraint);
};
