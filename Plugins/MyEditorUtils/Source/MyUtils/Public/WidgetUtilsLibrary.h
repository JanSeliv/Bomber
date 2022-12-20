// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Components/Widget.h"
//---
#include "WidgetUtilsLibrary.generated.h"

class UUserWidget;

/**
 * The common functions library for widgets.
 */
UCLASS()
class MYUTILS_API UWidgetUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Return the parent widget of a specific class in the widget tree hierarchy. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "InWidget", HidePin = "InWidget"))
	static UUserWidget* GetParentWidgetOfClass(const UUserWidget* InWidget, TSubclassOf<UUserWidget> ParentWidgetClass);

	/** Return the parent widget of a specific class in the widget tree hierarchy. */
	template <typename T>
	static FORCEINLINE T* GetParentWidgetOfClass(const UUserWidget* ChildWidget) { return Cast<T>(GetParentWidgetOfClass(ChildWidget, T::StaticClass())); }

	/** Returns first widget found by specified class iterating all widget objects. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static UUserWidget* FindWidgetOfClass(UObject* WorldContextObject, TSubclassOf<UUserWidget> ParentWidgetClass);

	/** Returns first widget found by specified class iterating all widget objects. */
	template <typename T>
	static FORCEINLINE T* FindWidgetOfClass(UObject* WorldContextObject) { return Cast<T>(FindWidgetOfClass(WorldContextObject, T::StaticClass())); }

	/** Returns the slate widget from UMG widget.
	* As an example, it returns SCheckbox slate widget from UCheckBox widget. */
	template <typename T>
	static TSharedPtr<T> GetSlateWidget(const UWidget* ForWidget) { return ForWidget ? StaticCastSharedPtr<T>(ForWidget->GetCachedWidget()) : nullptr; }
};
