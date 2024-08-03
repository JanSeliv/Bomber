﻿// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/NMMTypes.h"
#include "Subsystems/WorldSubsystem.h"
//---
#include "NMMCameraSubsystem.generated.h"

enum class ENMMState : uint8;

/**
 * Manages camera possessing and transitions in the Main Menu
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMCameraSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNMMOnCameraRailTransitionStateChange, ENMMCameraRailTransitionState, CameraRailState);
	
	/** Called when the Camera
	 * Rail End Transition is called
	 * Is local and not replicated. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FNMMOnCameraRailTransitionStateChange OnCameraRailTransitionStateChanged;
	
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMCameraSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns current camera component depending on the current Menu state. */
	UFUNCTION(BlueprintCallable, Category = "C++", DisplayName = "Find NNM Camera Component")
	static class UCameraComponent* FindCameraComponent(ENMMState MainMenuState);

	/** Returns attached Rail Camera of this spot that follows the camera to the next spot. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Current Rail Camera")
	static class ACameraActor* GetCurrentRailCamera();

	/** Returns attached Rail of this spot that follows the camera to the next spot. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Current Rail Rig")
	static class ACineCameraRigRail* GetCurrentRailRig();

	/** Starts viewing through camera of current cinematic or gameplay one depending on given state. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PossessCamera(ENMMState MainMenuState);

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called when the world is initialized. */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Return the stat id to use for this tickable. */
	virtual TStatId GetStatId() const override;

	/** Is called every frame to move the camera. */
	virtual void Tick(float DeltaTime) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(ENMMState NewState);

	/*********************************************************************************************
	 * Transitioning
	 ********************************************************************************************* */
public:
	/** Returns true if the camera should transit to the next spot, otherwise in backward direction. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Is NNM Camera Forward Transition")
	static bool IsCameraForwardTransition();

	/** Returns begin value, where the camera should start moving on the rail. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Camera Start Transition Value")
	static float GetCameraStartTransitionValue();

	/** Returns end value, where the camera should stop moving on the rail. */
	UFUNCTION(BlueprintPure, Category = "C++", DisplayName = "Get NNM Camera Last Transition Value")
	static float GetCameraLastTransitionValue();

	/** Applies the new state of camera rail transition state
	 * Is local and not replicated. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetNewCameraRailTranitionState(ENMMCameraRailTransitionState NewCameraRailState);
	
	/** Returns the current state of Camera rail transition. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE ENMMCameraRailTransitionState GetCurrentCameraRailTransitionState() const { return CameraRailTransitionStateInternal; }

protected:
	/** Is true during the transition when the camera is currently blending in to the rail or out from the Rail to the Spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Blending In/Out"))
	bool bIsBlendingInOutInternal = false;

	/** Is true when the rail of the camera is currently halfway of the current Spot to the next */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = " Is Rail Half Way Reached"))
	bool bIsRailHalfWayReached = false;

	/** Current camera transition state  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = " Camera Rail State"))
	ENMMCameraRailTransitionState CameraRailTransitionStateInternal = ENMMCameraRailTransitionState::None;

	/** Is called on starts blending the camera towards current spot on the rail. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBeginTransition();

	/** Is called on finishes blending the camera towards current spot on the rail. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndTransition();

	/** Is called in tick to update the camera transition when transitioning. */
	void TickTransition(float DeltaTime);
	
};
