// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Components/Widget.h"
//---
#include "WidgetUtilsLibrary.generated.h"

class UWidget;

/**
 * The common functions library for widgets.
 */
UCLASS()
class MYUTILS_API UWidgetUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Return the parent widget of a specific class in the widget tree hierarchy. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "InWidget"))
	static UWidget* GetParentWidgetOfClass(const UWidget* InWidget, TSubclassOf<UWidget> ParentWidgetClass);

	/** Return the parent widget of a specific class in the widget tree hierarchy. */
	template <typename T>
	static FORCEINLINE T* GetParentWidgetOfClass(const UWidget* ChildWidget) { return Cast<T>(GetParentWidgetOfClass(ChildWidget, T::StaticClass())); }

	/** Returns the slate widget from UMG widget.
	* As an example, it returns SCheckbox slate widget from UCheckBox widget. */
	template <typename T>
	static TSharedPtr<T> GetSlateWidget(const UWidget* ForWidget) { return ForWidget ? StaticCastSharedPtr<T>(ForWidget->GetCachedWidget()) : nullptr; }
};
