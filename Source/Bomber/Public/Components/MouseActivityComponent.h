// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "DataAssets/PlayerInputDataAsset.h" // FMouseVisibilitySettings
//---
#include "MouseActivityComponent.generated.h"

enum class ECurrentGameState : uint8;

class APlayerController;

/**
 * Component that responsible for mouse-related logic like showing and hiding itself.
 * Owner is Player Controller.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMouseActivityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UMouseActivityComponent();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMouseVisibilityChanged, bool, bIsShown);

	/** Called when mouse became shown or hidden. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnMouseVisibilityChanged OnMouseVisibilityChanged;

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Returns Player Controller of this component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	APlayerController* GetPlayerController() const;
	APlayerController& GetPlayerControllerChecked() const;

	/** Returns the mouse visibility settings according current game state. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FMouseVisibilitySettings& GetCurrentVisibilitySettings() const;

	/** Called to to set the mouse cursor visibility.
	 * @param bShouldShow true to show mouse cursor, otherwise hide it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseVisibility(bool bShouldShow);

	/** If true, set the mouse focus on game and UI, otherwise only focusing on game inputs. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseFocusOnUI(bool bFocusOnUI);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** How long the mouse is inactive at this moment. Is calculated in Tick if only inactivity is enabled.
	 * @see FMouseVisibilitySettings::bHideOnInactivity */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Inactive Time"))
	float CurrentlyInactiveSecInternal = 0.f;

	/** Cached settings for mouse visibility.
	 * @see UPlayerInputDataAsset::GetMouseVisibilitySettings() */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Visibility Settings"))
	TMap<ECurrentGameState, FMouseVisibilitySettings> VisibilitySettingsInternal;

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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMouseMove();

	/** Listen to toggle mouse visibility. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
