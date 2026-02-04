// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "BmrPowerupWidget.generated.h"

/**
 * Widget that represents the powerup powerup in the UI.
 */
UCLASS(Abstract)
class BOMBER_API UBmrPowerupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Updates the blends slider target to which widget will interpolate.
	 * @param NewValue The new value to set the slider to, should be in range [0, MaxValue].
	 * @param MaxValue The maximum value for the slider, used to display the percentage of the powerup level.
	 * @param bImmediateUpdate If true, the slider will be updated immediately without interpolation. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetTargetValue(float NewValue, float MaxValue, bool bImmediateUpdate = false);

	/** Updates the icon of the powerup powerup in the UI.
	 * @param InPowerupTag The tag of powerup powerup to display. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetPowerupIcon(FBmrPowerupTag InPowerupTag);

protected:
	/** Exposed property to be set in Details Panel of the type of powerup this UI or data element is associated with (e.g., Speed, BombCount, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design", meta = (BlueprintProtected, ExposeOnSpawn = "true"))
	FBmrPowerupTag PowerupTag = FBmrPowerupTag::None;

	/** The image UI widget used to display the icon of the power-up powerup.
	 * It automatically sets the icon based on the powerup PowerupTag property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UImage> PowerUpIcon = nullptr;

	/** Exposed property to be set in Details Panel of the duration of the interpolation when updating visual feedback (e.g., slider value change) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design", meta = (BlueprintProtected, ClampMin = "0.01"))
	float LerpDuration = 0.5f;

	/** The radial slider UI widget used to display or adjust the power-up level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class URadialSlider> RadialSlider = nullptr;

	/** Target value to interpolate toward when updating the slider */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	float TargetValue = 0.f;

	/** Time elapsed since starting the interpolation toward the new target value */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	float ElapsedLerpTime = 0.f;

	/** Whether the slider value needs to be updated based on target and elapsed time */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bNeedsUpdate = false;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called before the underlying slate widget is constructed to update widget at design time. */
	virtual void NativePreConstruct() override;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Is executed every tick when widget is enabled. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnLocalPawnReady(const struct FGameplayEventData& Payload);

	/** Is called when the associated powerup attribute is changed. */
	void OnPowerupAttributeChanged(const struct FOnAttributeChangeData& OnAttributeChangeData);
};