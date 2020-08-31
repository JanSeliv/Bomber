// Copyright 2020 Yevhenii Selivanov.

#include "LevelActorDataAsset.h"

// The empty mesh row
const FLevelActorMeshRow FLevelActorMeshRow::Empty = FLevelActorMeshRow();

// Default constructor
ULevelActorDataAsset::ULevelActorDataAsset()
{
	for (int32 It = 1; It < TO_FLAG(ELT::Max); It = 1 << It)
	{
		MeshesInternal.Emplace(FLevelActorMeshRow(TO_ENUM(ELevelType, It)));
	}
}

void ULevelActorDataAsset::GetMeshesByLevelType(TArray<FLevelActorMeshRow>& OutMeshes, const int32& LevelsTypesBitmask) const
{
	for (const FLevelActorMeshRow& MeshRowIt : MeshesInternal)
	{
		if (EnumHasAnyFlags(MeshRowIt.LevelType, TO_ENUM(ELevelType, LevelsTypesBitmask)))
		{
			OutMeshes.Emplace(MeshRowIt);
		}
	}
}

// Returns the first found mesh row that is equal by its level and item types to specified mesh row types.
void ULevelActorDataAsset::GetMeshRowByTypes(FLevelActorMeshRow& OutComparedMeshRow) const
{
	for (const FLevelActorMeshRow& MeshRowIt : MeshesInternal)
	{
		if (MeshRowIt.IsEqualTypes(OutComparedMeshRow))
		{
			OutComparedMeshRow = MeshRowIt;
			return;
		}
	}
}
