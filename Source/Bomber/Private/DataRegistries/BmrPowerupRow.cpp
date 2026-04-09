// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrPowerupRow.h"

// Bomber
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupRow)

// Finds powerup row data by powerup tag and level type, resolves current level type automatically if None
const FBmrPowerupRow* FBmrPowerupRow::GetRowByPowerupTag(FBmrPowerupTag Tag, EBmrLevelType LevelType /* = EBmrLevelType::None*/)
{
	if (LevelType == EBmrLevelType::None)
	{
		LevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	}

	return GetRowByPredicate([Tag, LevelType](const FBmrPowerupRow& Row)
	{
		return Row.PowerupTag == Tag
		       && (Row.LevelType == LevelType || Row.LevelType == ELT::Max);
	});
}
