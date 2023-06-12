// Copyright (c) Yevhenii Selivanov

#include "Engine/MyGameViewportClient.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/LocalPlayer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameViewportClient)

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

	for (ULocalPlayer* LocalPlayer : LocalPlayers)
	{
		constexpr float SquareAspectRatio = 1.f;
		const bool bIsWideScreen = NewAspectRatio > SquareAspectRatio;
		const EAspectRatioAxisConstraint NewAspectRatioAxisConstraint = bIsWideScreen ? AspectRatio_MaintainYFOV : AspectRatio_MaintainXFOV;
		LocalPlayer->AspectRatioAxisConstraint = NewAspectRatioAxisConstraint;
	}

	if (LastUpdatedAspectRatioInternal != NewAspectRatio)
	{
		if (LastUpdatedAspectRatioInternal > 0.f) // do not broadcast on first update
		{
			OnAspectRatioChanged.Broadcast(NewAspectRatio);
		}

		LastUpdatedAspectRatioInternal = NewAspectRatio;
	}
}
