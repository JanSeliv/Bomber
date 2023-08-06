// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/GameViewportClient.h"
//---
#include "MyGameViewportClient.generated.h"

enum EAspectRatioAxisConstraint : int;

/**
 * Is the engine's interface to a game viewport.
 * Implements parent to have more control on input events.
 */
UCLASS()
class BOMBER_API UMyGameViewportClient final : public UGameViewportClient
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAspectRatioChanged, float, NewAspectRatio, EAspectRatioAxisConstraint, NewAxisConstraint);

	/** Called when the aspect ratio has been changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnAspectRatioChanged OnAspectRatioChanged;

	/** Returns the Axis Constraint of the viewport based on current aspect ratio.
	 * Alternative, UUtilsLibrary::GetViewportAspectRatioAxisConstraint() can be used. */
	UFUNCTION(BlueprintPure, Category = "C++")
	TEnumAsByte<EAspectRatioAxisConstraint> GetAxisConstraint() const;

	/** Returns the last updated aspect ratio. */
	UFUNCTION(BlueprintPure, Category = "C++")
	int32 GetAspectRatio() const { return LastUpdatedAspectRatioInternal; }

	/** Gets whether or not the cursor should always be locked to the viewport. */
	virtual bool ShouldAlwaysLockMouse() override { return true; }

	/** Is called on applying different video settings like changing resolution and enabling fullscreen mode. */
	virtual void RedrawRequested(FViewport* InViewport) override;

	/** Dynamically changes aspect ratio constraint to support all screens like ultra-wide and vertical one.
	 * It strongly affects how FOV is calculated and is called during viewport redraw request.
	 * By Epic's default, only X-constrain was used, so only horizontal FOV was calculated, that makes ultra-wide to be unsupported.
	 * This function makes it different:
	 * Use Y-constrain (vertical FOV) for wide screens (16:9, ultra-wide 21:9 etc).
	 * Use X-constrain (horizontal FOV) for vertical screens (9:16 etc) and square screens (1:1). */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateAspectRatio();

protected:
	/** Cached data about last broadcasted aspect ratio. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Last c Aspect Ratio"))
	float LastUpdatedAspectRatioInternal = 0.f;
};
