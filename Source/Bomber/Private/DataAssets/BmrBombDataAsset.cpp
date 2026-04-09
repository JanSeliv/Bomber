// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrBombDataAsset.h"

// Bomber
#include "DalSubsystem.h"
#include "DataRegistries/BmrBombRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBombDataAsset)

// Default constructor
UBmrBombDataAsset::UBmrBombDataAsset()
{
	ActorType = EAT::Bomb;
	RowType = FBmrBombRow::StaticStruct();
}

// Returns the bomb data asset
const UBmrBombDataAsset& UBmrBombDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Finds bomb row data by instigator actor, resolves level type from its MapComponent or SkeletalMeshComponent
const FBmrBombRow& UBmrBombDataAsset::GetBombRow(const AActor* InInstigator)
{
	return FBmrBombRow::GetBombRow(InInstigator);
}

// Returns the amount of unique bomb materials from Data Registry
int32 UBmrBombDataAsset::GetBombMaterialsNum()
{
	return FBmrBombRow::GetBombMaterialsNum();
}

// Returns the bomb material by specified index from Data Registry, or nullptr
UMaterialInterface* UBmrBombDataAsset::GetBombMaterial(int32 Index)
{
	return FBmrBombRow::GetBombMaterial(Index);
}
