// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrPowerupRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupRow)

// Finds powerup row data by powerup tag from Data Registry
const FBmrPowerupRow* FBmrPowerupRow::GetRowByPowerupTag(FBmrPowerupTag Tag)
{
	return GetRowByPredicate([Tag](const FBmrPowerupRow& Row)
	{
		return Row.PowerupTag == Tag;
	});
}
