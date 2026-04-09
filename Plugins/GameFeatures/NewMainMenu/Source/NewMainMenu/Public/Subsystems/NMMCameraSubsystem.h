// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/ModularGameFeaturePluginSubsystem.h"

// NMM
#include "Data/NMMTypes.h" // ENMMCameraRailTransitionState

#include "NMMCameraSubsystem.generated.h"

/**
 * Manages camera possessing and transitions in the Main Menu
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNMMCameraSubsystem : public UModularGameFeaturePluginSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNMMOnCameraRailTransitionStateChange, ENMMCameraRailTransitionState, CameraRailState);

	/** Called when the Camera
	 * Rail End Transition is called
	 * Is local and not replicated. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[NewMainMenu]")
	FNMMOnCameraRailTransitionStateChange OnCameraRailTransitionStateChanged;

	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMCameraSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns current camera component depending on the current Menu state. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", DisplayName = "Find NNM Camera Component")
	static class UCameraComponent* FindCameraComponent(struct FNmmStateTag MenuState);

	/** Returns attached Rail Camera of this spot that follows the camera to the next spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Current Rail Camera")
	static class ACameraActor* GetCurrentRailCamera();

	/** Returns attached Rail of this spot that follows the camera to the next spot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Current Rail Rig")
	static class ACineCameraRigRail* GetCurrentRailRig();

	/** Starts viewing through camera of current cinematic or gameplay one depending on given state. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void PossessCamera(FNmmStateTag MenuState);

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** Enables ticking for camera transitions */
	virtual FORCEINLINE bool IsTickable() const override { return true; }

protected:
	/** Subscribes to menu state events */
	virtual void OnGameFeatureInitialize_Implementation() override;

	/** Clears all transient data contained in this subsystem. */
	virtual void OnGameFeatureDeinitialize_Implementation() override;

	/** Is called every frame to move the camera. */
	virtual void Tick(float DeltaTime) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(const struct FGameplayEventData& Payload);

	/** Called when a cinematic spot finished loading, possesses camera if it was deferred. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnActiveMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);

	/*********************************************************************************************
	 * Transitioning
	 ********************************************************************************************* */
public:
	/** Returns true if the camera should transit to the next spot, otherwise in backward direction. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Is NNM Camera Forward Transition")
	static bool IsCameraForwardTransition();

	/** Returns begin value, where the camera should start moving on the rail. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Camera Start Transition Value")
	static float GetCameraStartTransitionValue();

	/** Returns end value, where the camera should stop moving on the rail. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]", DisplayName = "Get NNM Camera Last Transition Value")
	static float GetCameraLastTransitionValue();

	/** Applies the new state of camera rail transition state
	 * Is local and not replicated. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetNewCameraRailTransitionState(ENMMCameraRailTransitionState NewCameraRailState);

	/** Returns the current state of Camera rail transition. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE ENMMCameraRailTransitionState GetCurrentCameraRailTransitionState() const { return CameraRailTransitionState; }

protected:
	/** Is true during the transition when the camera is currently blending in to the rail or out from the Rail to the Spot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	bool bIsBlendingInOut = false;

	/** Current camera transition state  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	ENMMCameraRailTransitionState CameraRailTransitionState = ENMMCameraRailTransitionState::None;

	/** Is called on starts blending the camera towards current spot on the rail. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnBeginTransition();

	/** Is called on finishes blending the camera towards current spot on the rail. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnEndTransition();

	/** Is called in tick to update the camera transition when transitioning. */
	void TickTransition(float DeltaTime);
};
