// Copyright (c) Yevhenii Selivanov

#include "Engine/MyGameViewportClient.h"
//---
#include "Engine/LocalPlayer.h"

// Is called on applying different video settings like changing resolution and enabling fullscreen mode
void UMyGameViewportClient::RedrawRequested(FViewport* InViewport)
{
	Super::RedrawRequested(InViewport);

	UpdateAspectRatioAxisConstraint();
}

// Dynamically changes aspect ratio constraint to support all screens like ultra-wide and vertical one
void UMyGameViewportClient::UpdateAspectRatioAxisConstraint()
{
	const TArray<ULocalPlayer*>& LocalPlayers = GetOuterUEngine()->GetGamePlayers(this);
	const FIntPoint ViewportResolution = Viewport ? Viewport->GetSizeXY() : FIntPoint::ZeroValue;
	if (LocalPlayers.IsEmpty()
	    || ViewportResolution == FIntPoint::ZeroValue)
	{
		return;
	}

	const float NewAspectRatio = static_cast<float>(ViewportResolution.X) / static_cast<float>(ViewportResolution.Y);

	bool bAspectRatioChangedAtLeastOnce = false;
	for (ULocalPlayer* LocalPlayer : LocalPlayers)
	{
		constexpr float SquareAspectRatio = 1.f;
		const bool bIsWideScreen = NewAspectRatio > SquareAspectRatio;
		const EAspectRatioAxisConstraint NewAspectRatioAxisConstraint = bIsWideScreen ? AspectRatio_MaintainYFOV : AspectRatio_MaintainXFOV;
		if (NewAspectRatio != LocalPlayer->AspectRatioAxisConstraint)
		{
			LocalPlayer->AspectRatioAxisConstraint = NewAspectRatioAxisConstraint;
			bAspectRatioChangedAtLeastOnce = true;
		}
	}

	if (bAspectRatioChangedAtLeastOnce)
	{
		OnAspectRatioChanged.Broadcast(NewAspectRatio);
	}
}
