// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "UtilsLibrary.generated.h"

class UWidget;

/**
 * The common functions library
 */
UCLASS()
class MYUTILS_API UUtilsLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Return the parent widget of a specific class in the widget tree hierarchy. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "InWidget"))
	static UWidget* GetParentWidgetOfClass(const UWidget* InWidget, TSubclassOf<UWidget> ParentWidgetClass);

	/** Return the parent widget of a specific class in the widget tree hierarchy. */
	template <typename T>
	static FORCEINLINE T* GetParentWidgetOfClass(const UWidget* ChildWidget) { return Cast<T>(GetParentWidgetOfClass(ChildWidget, T::StaticClass())); }
};
