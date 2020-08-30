// Copyright 2020 Yevhenii Selivanov.

#include "LevelActorDataAsset.h"

// Default constructor
ULevelActorDataAsset::ULevelActorDataAsset()
{
	for (int32 It = 1; It < TO_FLAG(LT::Max); It = 1 << It)
	{
		MeshesInternal.Emplace(FLevelActorMeshRow(TO_ENUM(ELevelType, It)));
	}
}

void ULevelActorDataAsset::GetMeshesByLevelType(TArray<FLevelActorMeshRow>& OutMeshes, const int32& LevelsTypesBitmask) const
{
	for (const FLevelActorMeshRow& MeshRowIt : MeshesInternal)
	{
		if(EnumHasAnyFlags(MeshRowIt.LevelType, TO_ENUM(ELevelType, LevelsTypesBitmask)))
		{
			OutMeshes.Emplace(MeshRowIt);
		}
	}
}
