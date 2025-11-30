// Copyright (c) Yevhenii Selivanov

#include "Engine/BmrGameViewportClient.h"

// Bomber
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameViewportClient)

// Returns the Axis Constraint of the viewport based on current aspect ratio
TEnumAsByte<EAspectRatioAxisConstraint> UBmrGameViewportClient::GetAxisConstraint() const
{
	constexpr float SquareAspectRatio = 1.f;
	const bool bIsWideScreen = LastUpdatedAspectRatio > SquareAspectRatio;
	return bIsWideScreen ? AspectRatio_MaintainYFOV : AspectRatio_MaintainXFOV;
}

// Is called on applying different video settings like changing resolution and enabling fullscreen mode
void UBmrGameViewportClient::RedrawRequested(FViewport* InViewport)
{
	Super::RedrawRequested(InViewport);

	UpdateAspectRatio();
}

// Dynamically changes aspect ratio constraint to support all screens like ultra-wide and vertical one
void UBmrGameViewportClient::UpdateAspectRatio()
{
	const TArray<ULocalPlayer*>& LocalPlayers = GEngine->GetGamePlayers(this);
	const FIntPoint ViewportResolution = UUtilsLibrary::GetViewportResolution();
	if (LocalPlayers.IsEmpty()
	    || ViewportResolution == FIntPoint::ZeroValue)
	{
		return;
	}

	const float NewAspectRatio = static_cast<float>(ViewportResolution.X) / static_cast<float>(ViewportResolution.Y);
	const bool bIsAspectRatioChanged = LastUpdatedAspectRatio != NewAspectRatio;
	LastUpdatedAspectRatio = NewAspectRatio;

	const TEnumAsByte<EAspectRatioAxisConstraint> AxisConstraint = GetAxisConstraint();
	for (ULocalPlayer* LocalPlayer : LocalPlayers)
	{
		LocalPlayer->AspectRatioAxisConstraint = AxisConstraint;
	}

	if (bIsAspectRatioChanged
	    && LastUpdatedAspectRatio > 0.f) // do not broadcast on first update
	{
		OnAspectRatioChanged.Broadcast(NewAspectRatio, AxisConstraint);
	}
}
