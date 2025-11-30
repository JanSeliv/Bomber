// Copyright (c) Yevhenii Selivanov.

#include "Structures/BmrManageableWidgetData.h"

// UE
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrManageableWidgetData)

// Contains default widget data with no values set
const FBmrManageableWidgetData& FBmrManageableWidgetData::Empty = FBmrManageableWidgetData();

// Returns true if all data is setup correctly
bool FBmrManageableWidgetData::IsValid() const
{
	return WidgetClass && WidgetTag.IsValid();
}

// Returns compact string representation of this widget data
FString FBmrManageableWidgetData::ToString() const
{
	return FString::Printf(TEXT("WidgetClass: %s, WidgetTag: %s"), *GetNameSafe(WidgetClass), *WidgetTag.ToString());
}