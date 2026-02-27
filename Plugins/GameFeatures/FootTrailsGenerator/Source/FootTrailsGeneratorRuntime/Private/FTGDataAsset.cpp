// Copyright (c) Yevhenii Selivanov

#include "FTGDataAsset.h"

#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FTGDataAsset)

const UFTGDataAsset& UFTGDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}
