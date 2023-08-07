// Copyright (c) Yevhenii Selivanov

#include "Data/NMMDataAsset.h"
//---
#include "NMMUtils.h"
#include "Components/NMMHUDComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNMMDataAsset& UNMMDataAsset::Get()
{
	const UNMMHUDComponent* HUDComponent = UNMMUtils::GetHUDComponent();
	checkf(HUDComponent, TEXT("ERROR: 'HUDComponent' is not initialized yet!"));
	const UNMMDataAsset* DataAsset = HUDComponent->GetNewMainMenuDataAsset();
	checkf(DataAsset, TEXT("%s: 'DataAsset' is not set"), *FString(__FUNCTION__));
	return *DataAsset;
}
