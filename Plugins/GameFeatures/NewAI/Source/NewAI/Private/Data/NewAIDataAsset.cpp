// Copyright (c) Yevhenii Selivanov

#include "Data/NewAIDataAsset.h"
//---
#include "Subsystems/NewAIBaseSubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNewAIDataAsset& UNewAIDataAsset::Get(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UNewAIDataAsset* DataAsset = UNewAIBaseSubsystem::Get(OptionalWorldContext).GetNewAIDataAsset();
	checkf(DataAsset, TEXT("%s: 'DataAsset' is not set"), *FString(__FUNCTION__));
	return *DataAsset;
}
