// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrMeshData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMeshData)

// The empty data
const FBmrMeshData FBmrMeshData::Empty = FBmrMeshData();

// Constructor that initializes the data directly
FBmrMeshData::FBmrMeshData(const class UBmrLevelActorRow* InPlayerRow, int32 InSkinIndex /* = 0*/)
    : Row(InPlayerRow)
    , SkinIndex(InSkinIndex)
{
}