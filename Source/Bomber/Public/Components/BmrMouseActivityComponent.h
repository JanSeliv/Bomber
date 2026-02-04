// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

// Bomber
#include "Structures/BmrMouseVisibilitySettings.h"

#include "BmrMouseActivityComponent.generated.h"

enum class EBmrCurrentGameState : uint8;

class APlayerController;

/**
 * Component that responsible for mouse-related logic like showing and hiding itself.
 * Owner is Player Controller.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UBmrMouseActivityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UBmrMouseActivityComponent();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMouseVisibilityChanged, bool, bIsShown);

	/** Called when mouse became shown or hidden. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnMouseVisibilityChanged OnMouseVisibilityChanged;

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Returns Player Controller of this component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	APlayerController* GetPlayerController() const;
	APlayerController& GetPlayerControllerChecked() const;

	/** Returns current mouse visibility settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrMouseVisibilitySettings& GetCurrentVisibilitySettings() const;

	/** Applies the new mouse visibility settings by game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMouseVisibilitySettingsEnabled(bool bEnable, EBmrCurrentGameState GameState);

	/** Applies the new mouse visibility settings by custom game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMouseVisibilitySettingsEnabledCustom(bool bEnable, FName CustomGameState);

	/*********************************************************************************************
	 * Protected functions
	 * Use SetMouseVisibilitySettingsEnabled and SetMouseVisibilitySettingsEnabledCustom instead.
	 ********************************************************************************************* */
public:
	/** Applies the new mouse visibility settings. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void EnableMouseVisibilitySettings(const FBmrMouseVisibilitySettings& NewSettings);

	/** Restores previous mouse visibility settings. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void DisableMouseVisibilitySettings();

	/** Called to to set the mouse cursor visibility.
	 * @param bShouldShow true to show mouse cursor, otherwise hide it. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMouseVisibility(bool bShouldShow);

	/** If true, set the mouse focus on game and UI, otherwise only focusing on game inputs. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMouseFocusOnUI(bool bFocusOnUI);

	/** Is called in tick to detect mouse movement and handle inactivity. */
	void TickHandleInactivity(float DeltaTime);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** How long the mouse is inactive at this moment. Is calculated in Tick if only inactivity is enabled.
	 * @see FMouseVisibilitySettings::bHideOnInactivity */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	float CurrentlyInactiveSec = 0.f;

	/** Currently used mouse visibility settings. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrMouseVisibilitySettings CurrentVisibilitySettings = FBmrMouseVisibilitySettings::Invalid;

	/** Last mouse visibility settings, used for restoring the previous state when disabling the current one. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrMouseVisibilitySettings PreviousVisibilitySettings = FBmrMouseVisibilitySettings::Invalid;

	/** Last mouse position to detect inactivity. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FVector2D LastMousePosition = FVector2D::ZeroVector;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Called every frame to calculate Delta Time. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called from 'Mouse Move' input action to show inactive mouse. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnMouseMove();

	/** Listen to toggle mouse visibility. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);
};