// Copyright (c) Yevhenii Selivanov

#include "UtilsLibrary.h"
//---
#include "Blueprint/WidgetTree.h"

// Return the parent widget of a specific class in the widget tree hierarchy
UWidget* UUtilsLibrary::GetParentWidgetOfClass(const UWidget* InWidget, TSubclassOf<UWidget> ParentWidgetClass)
{
	UObject* It = InWidget ? InWidget->GetParent() : nullptr;
	if (!It)
	{
		return nullptr;
	}

	UWidget* FoundWidget = nullptr;
	while ((It = It->GetOuter()) != nullptr)
	{
		// Check every outer until the desired one is found
		if (It->IsA(ParentWidgetClass))
		{
			FoundWidget = Cast<UWidget>(It);
			break;
		}

		if (!It->IsA<UWidgetTree>()
			&& !It->IsA<UWidget>())
		{
			// No sense to iterate non-widget outers
			break;
		}
	}

	return FoundWidget;
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
