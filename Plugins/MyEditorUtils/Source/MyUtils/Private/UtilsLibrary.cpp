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
