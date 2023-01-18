// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/GameViewportClient.h"

// Returns true if viewport is initialized, is always true in PIE, but takes a while in builds
bool UUtilsLibrary::IsViewportInitialized()
{
	UGameViewportClient* GameViewport = GEngine ? GEngine->GameViewport : nullptr;
	FViewport* Viewport = GameViewport ? GameViewport->Viewport : nullptr;
	if (!Viewport)
	{
		return false;
	}

	auto IsZeroViewportSize = [Viewport] { return Viewport->GetSizeXY() == FIntPoint::ZeroValue; };

	if (IsZeroViewportSize())
	{
		// Try update its value by mouse enter event
		GameViewport->MouseEnter(Viewport, FIntPoint::ZeroValue.X, FIntPoint::ZeroValue.Y);
		return !IsZeroViewportSize();
	}

	return true;
}

// Returns 'MaintainYFOV' if Horizontal FOV is currently used while 'MaintainXFOV' for the Vertical one
TEnumAsByte<EAspectRatioAxisConstraint> UUtilsLibrary::GetViewportAspectRatioAxisConstraint()
{
	const APlayerController* PlayerController = GEngine ? GEngine->GetFirstLocalPlayerController(GWorld) : nullptr;
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->AspectRatioAxisConstraint : TEnumAsByte(AspectRatio_MAX);
}
