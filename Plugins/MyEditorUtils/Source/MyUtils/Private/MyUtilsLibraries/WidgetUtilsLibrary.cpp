// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
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

// Returns first widget by specified class iterating all widget objects
UUserWidget* FWidgetUtilsLibrary::FindWidgetOfClass(UObject* WorldContextObject, TSubclassOf<UUserWidget> ParentWidgetClass)
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(WorldContextObject, /*out*/FoundWidgets, ParentWidgetClass);
	return !FoundWidgets.IsEmpty()? FoundWidgets[0] : nullptr;
}

