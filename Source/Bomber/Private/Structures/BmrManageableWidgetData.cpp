// Copyright (c) Yevhenii Selivanov.

#include "Structures/ManageableWidgetData.h"

// UE
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ManageableWidgetData)

// Contains default widget data with no values set
const FManageableWidgetData& FManageableWidgetData::Empty = FManageableWidgetData();

// Returns true if all data is setup correctly
bool FManageableWidgetData::IsValid() const
{
	return WidgetClass && WidgetTag.IsValid();
}

// Returns compact string representation of this widget data
FString FManageableWidgetData::ToString() const
{
	return FString::Printf(TEXT("WidgetClass: %s, WidgetTag: %s"), *GetNameSafe(WidgetClass), *WidgetTag.ToString());
}