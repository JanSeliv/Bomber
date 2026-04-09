// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrWidgetRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrWidgetRow)

// Returns widget row data by tag from Data Registry, or nullptr
const FBmrWidgetRow* FBmrWidgetRow::GetRowByTag(FGameplayTag WidgetTag)
{
	return GetRowByPredicate([WidgetTag](const FBmrWidgetRow& Row)
	{
		return Row.WidgetTag == WidgetTag;
	});
}