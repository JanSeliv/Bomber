// Copyright (c) Yevhenii Selivanov

#include "Data/PasDataAsset.h"

// DAL
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PasDataAsset)

// Returns PlayAreaSurrounder data asset or crash when cannot be obtained
const UPasDataAsset& UPasDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}