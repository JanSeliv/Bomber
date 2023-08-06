// Copyright (c) Yevhenii Selivanov

#include "Engine/MyGameViewportClient.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/LocalPlayer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameViewportClient)

// Returns the Axis Constraint of the viewport based on current aspect ratio
TEnumAsByte<EAspectRatioAxisConstraint> UMyGameViewportClient::GetAxisConstraint() const
{
	constexpr float SquareAspectRatio = 1.f;
	const bool bIsWideScreen = LastUpdatedAspectRatioInternal > SquareAspectRatio;
	return bIsWideScreen ? AspectRatio_MaintainYFOV : AspectRatio_MaintainXFOV;
}

// Is called on applying different video settings like changing resolution and enabling fullscreen mode
void UMyGameViewportClient::RedrawRequested(FViewport* InViewport)
{
	Super::RedrawRequested(InViewport);

	UpdateAspectRatio();
}

// Dynamically changes aspect ratio constraint to support all screens like ultra-wide and vertical one
void UMyGameViewportClient::UpdateAspectRatio()
{
	const TArray<ULocalPlayer*>& LocalPlayers = GetOuterUEngine()->GetGamePlayers(this);
	const FIntPoint ViewportResolution = UUtilsLibrary::GetViewportResolution();
	if (LocalPlayers.IsEmpty()
	    || ViewportResolution == FIntPoint::ZeroValue)
	{
		return;
	}

	const float NewAspectRatio = static_cast<float>(ViewportResolution.X) / static_cast<float>(ViewportResolution.Y);
	const bool bIsAspectRatioChanged = LastUpdatedAspectRatioInternal != NewAspectRatio;
	LastUpdatedAspectRatioInternal = NewAspectRatio;

	const TEnumAsByte<EAspectRatioAxisConstraint> AxisConstraint = GetAxisConstraint();
	for (ULocalPlayer* LocalPlayer : LocalPlayers)
	{
		LocalPlayer->AspectRatioAxisConstraint = AxisConstraint;
	}

	if (bIsAspectRatioChanged
	    && LastUpdatedAspectRatioInternal > 0.f) // do not broadcast on first update
	{
		OnAspectRatioChanged.Broadcast(NewAspectRatio, AxisConstraint);
	}
}
