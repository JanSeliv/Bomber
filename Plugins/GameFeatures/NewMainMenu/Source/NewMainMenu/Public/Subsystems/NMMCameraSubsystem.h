// Copyright (c) Yevhenii Selivanov

#pragma once

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
protected:
	/** Is called on starts blending the camera towards current spot on the rail. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBeginTransition();

	/** Is called on finishes blending the camera towards current spot on the rail. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndTransition();

	/** Is called in tick to update the camera transition when transitioning. */
	void TickTransition(float DeltaTime);
};
