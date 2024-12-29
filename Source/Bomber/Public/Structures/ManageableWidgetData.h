// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Templates/SubclassOf.h"
//---
#include "ManageableWidgetData.generated.h"

/**
* Default data for manageable widgets to be set in the UI data assets.
* Its data is expected to be passed in UWidgetsSubsystem::Get().CreateManageableWidget(Data);
 */
USTRUCT(BlueprintType)
struct BOMBER_API FManageableWidgetData
{
	GENERATED_BODY()

	/** The class of the widget to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TSubclassOf<class UUserWidget> WidgetClass = nullptr;

	/** If true, adds the widget to the viewport, so it will be registered in slate and ready to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	bool bAddToViewport = true;

	/** The Z-order of the widget, higher order will be drawn on top of lower order. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 ZOrder = 0;
};