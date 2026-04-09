// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrPlayerPropRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerPropRow)

// Gathers all player prop rows for the given player tag from Data Registry
void FBmrPlayerPropRow::GetPlayerProps(const FBmrPlayerTag& PlayerTag, TArray<const FBmrPlayerPropRow*>& OutProps)
{
	GetRowsByPredicate(OutProps, [&PlayerTag](const FBmrPlayerPropRow& Row)
	{
		return Row.PlayerTag == PlayerTag;
	});
}