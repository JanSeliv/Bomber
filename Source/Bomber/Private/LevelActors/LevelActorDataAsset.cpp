// Copyright 2020 Yevhenii Selivanov.

#include "LevelActorDataAsset.h"

// Default constructor
ULevelActorDataAsset::ULevelActorDataAsset()
{
	for (ELevelType LevelTypeIt : TEnumRange<ELevelType>())
	{
		Meshes.Emplace(FLevelActorMeshRow(LevelTypeIt));
	}
}

