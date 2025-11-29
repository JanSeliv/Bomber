// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"

#include "ManageableWidgetData.generated.h"

/**
 * Default data for manageable widgets to be set in the UI data assets.
 * Its data is expected to be passed in UWidgetsSubsystem::Get().CreateManageableWidget(Data);
 */
USTRUCT(BlueprintType)
struct BOMBER_API FManageableWidgetData
{
	GENERATED_BODY()

	/** Contains default widget data with no values set. */
	static const FManageableWidgetData& Empty;

	/** The class of the widget to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TSubclassOf<class UUserWidget> WidgetClass = nullptr;

	/** The tag associated with this widget, is used for obtaining its widget data or widget instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (Categories = "UI"))
	FGameplayTag WidgetTag = FGameplayTag::EmptyTag;

	/** If true, adds the widget to the viewport, so it will be registered in slate and ready to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	bool bAddToViewport = true;

	/** The Z-order of the widget, higher order will be drawn on top of lower order. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 ZOrder = 0;

	/** Returns true if all data is setup correctly. */
	bool IsValid() const;

	/** Returns compact string representation of this widget data. */
	FString ToString() const;

	/** Operators for finding widget data by tag. */
	friend BOMBER_API bool operator==(const FManageableWidgetData& A, FGameplayTag B) { return A.WidgetTag == B; }
};

/**
 * A container struct to hold multiple manageable widgets, which usually are created dynamically and associated with specific UI tag.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrManageableWidgetsContainer
{
	GENERATED_BODY()

	/** Widget instances managed by this container. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TArray<TObjectPtr<UUserWidget>> WidgetInstances;
};