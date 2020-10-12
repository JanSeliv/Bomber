// Copyright 2020 Yevhenii Selivanov.

#include "Globals/LevelActorDataAsset.h"

void ULevelActorDataAsset::GetRowsByLevelType(TArray<ULevelActorRow*>& OutRows, int32 LevelsTypesBitmask) const
{
	for (ULevelActorRow* const& RowIt : RowsInternal)
	{
		if (RowIt
			&& RowIt->Mesh //is not empty
			&& EnumHasAnyFlags(RowIt->LevelType, TO_ENUM(ELevelType, LevelsTypesBitmask)))
		{
			OutRows.Emplace(RowIt);
		}
	}
}
