// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"

// Return the parent widget of a specific class in the widget tree hierarchy
UUserWidget* FWidgetUtilsLibrary::GetParentWidgetOfClass(const UUserWidget* InWidget, TSubclassOf<UUserWidget> ParentWidgetClass)
{
	UObject* It = InWidget ? InWidget->GetParent() : nullptr;
	if (!It)
	{
		return nullptr;
	}

	UUserWidget* FoundWidget = nullptr;
	while ((It = It->GetOuter()) != nullptr)
	{
		// Check every outer until the desired one is found
		if (It->IsA(ParentWidgetClass))
		{
			FoundWidget = Cast<UUserWidget>(It);
			break;
		}

		if (!It->IsA<UWidgetTree>()
			&& !It->IsA<UUserWidget>())
		{
			// No sense to iterate non-widget outers
			break;
		}
	}

	return FoundWidget;
}

// Returns first child widget found by specified class iterating all widget objects
UUserWidget* FWidgetUtilsLibrary::GetChildWidgetOfClass(const UUserWidget* ParentWidget, TSubclassOf<UUserWidget> ChildWidgetClass)
{
	if (!ParentWidget)
	{
		return nullptr;
	}

	TArray<UWidget*> ChildWidgets;
	ParentWidget->WidgetTree->GetAllWidgets(ChildWidgets);

	// Iterate through all child widgets to find the one of the specified class
	for (UWidget* Widget : ChildWidgets)
	{
		UUserWidget* UserWidget = Cast<UUserWidget>(Widget);
		if (UserWidget && UserWidget->IsA(ChildWidgetClass))
		{
			return UserWidget;
		}
	}

	return nullptr;
}

// Returns first widget by specified class iterating all widget objects
UUserWidget* FWidgetUtilsLibrary::FindWidgetOfClass(UObject* WorldContextObject, TSubclassOf<UUserWidget> ParentWidgetClass)
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(WorldContextObject, /*out*/FoundWidgets, ParentWidgetClass);
	return !FoundWidgets.IsEmpty() ? FoundWidgets[0] : nullptr;
}

// Completely destroys specified widget
void FWidgetUtilsLibrary::DestroyWidget(UUserWidget& ParentWidget)
{
	// Get an array of all child widgets
	TArray<UWidget*> ChildWidgets;
	const UWidgetTree* WidgetTree = ParentWidget.WidgetTree;
	WidgetTree->GetAllWidgets(ChildWidgets);

	// Iterate through the child widgets
	for (UWidget* ChildWidgetIt : ChildWidgets)
	{
		UUserWidget* ChildUserWidget = Cast<UUserWidget>(ChildWidgetIt);
		const UWidgetTree* WidgetTreeIt = ChildUserWidget ? ChildUserWidget->WidgetTree : nullptr;
		const bool bHasChildWidgets = WidgetTreeIt && WidgetTreeIt->RootWidget;

		if (bHasChildWidgets)
		{
			// If the child widget has its own child widgets, recursively remove and destroy them
			DestroyWidget(*ChildUserWidget);
		}
	}

	// Hide widget to let last chance react on visibility change
	ParentWidget.SetVisibility(ESlateVisibility::Collapsed);

	// Remove the child widget from the viewport
	ParentWidget.RemoveFromParent();

	// RemoveFromParent() does not completely destroy widget, so schedule the child widget for destruction
	ParentWidget.ConditionalBeginDestroy();
}

// Is alternative to Engine's CreateWidget with build-in add to viewport functionality
UUserWidget* FWidgetUtilsLibrary::CreateWidgetByClass(TSubclassOf<UUserWidget> WidgetClass, bool bAddToViewport/*= true*/, int32 ZOrder/* = 0*/, const UObject* OptionalWorldContext/* = nullptr*/)
{
	UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	if (!ensureMsgf(World, TEXT("ASSERT: [%i] %hs:\n'World' is not valid!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(WidgetClass, TEXT("ASSERT: [%i] %hs:\n'WidgetClass' is not valid!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	UUserWidget* CreatedWidget = CreateWidget(World, WidgetClass);
	checkf(CreatedWidget, TEXT("ERROR: [%i] %hs:\n'CreatedWidget' is null!"), __LINE__, __FUNCTION__);

	if (bAddToViewport)
	{
		CreatedWidget->AddToViewport(ZOrder);
	}

	return CreatedWidget;
}