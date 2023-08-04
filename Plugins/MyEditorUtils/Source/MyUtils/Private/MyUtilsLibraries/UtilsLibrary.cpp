// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "UnrealClient.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
//---
#include "Engine/Engine.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif // WITH_EDITOR
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(UtilsLibrary)

// Checks, is the current world placed in the editor
bool UUtilsLibrary::IsEditor()
{
#if WITH_EDITOR
	return FEditorUtilsLibrary::IsEditor();
#endif
	return false;
}

// Checks is the current world placed in the editor and the game not started yet
bool UUtilsLibrary::IsEditorNotPieWorld()
{
#if WITH_EDITOR
	return FEditorUtilsLibrary::IsEditorNotPieWorld();
#endif
	return false;
}

// Returns true if game is started in the Editor
bool UUtilsLibrary::IsPIE()
{
#if WITH_EDITOR
	return FEditorUtilsLibrary::IsPIE();
#endif
	return false;
}

// Returns true if is started multiplayer game (server + client(s)) right in the Editor
bool UUtilsLibrary::IsEditorMultiplayer()
{
#if WITH_EDITOR
	return FEditorUtilsLibrary::IsEditorMultiplayer();
#endif
	return false;
}

// Returns the index of current player during editor multiplayer
int32 UUtilsLibrary::GetEditorPlayerIndex()
{
#if WITH_EDITOR
	return FEditorUtilsLibrary::IsEditorMultiplayer();
#endif
	return INDEX_NONE;
}

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

// Returns the actual screen resolution
FIntPoint UUtilsLibrary::GetViewportResolution()
{
	const FViewport* Viewport = IsViewportInitialized() ? GEngine->GameViewport->Viewport : nullptr;

#if WITH_EDITOR
	if (FEditorUtilsLibrary::IsEditor()
		&& !Viewport)
	{
		Viewport = FEditorUtilsLibrary::GetEditorViewport();
	}
#endif

	return Viewport ? Viewport->GetSizeXY() : FIntPoint::ZeroValue;
}

// Returns 'MaintainYFOV' if Horizontal FOV is currently used while 'MaintainXFOV' for the Vertical one
TEnumAsByte<EAspectRatioAxisConstraint> UUtilsLibrary::GetViewportAspectRatioAxisConstraint()
{
	const APlayerController* PlayerController = GEngine ? GEngine->GetFirstLocalPlayerController(GWorld) : nullptr;
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->AspectRatioAxisConstraint : TEnumAsByte(AspectRatio_MAX);
}
