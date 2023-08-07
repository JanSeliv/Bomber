// Copyright (c) Yevhenii Selivanov

#include "Data/NewMainMenuDataAsset.h"
//---
#include "NewMainMenuUtils.h"
#include "Components/NewMainMenuHUDComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNewMainMenuDataAsset& UNewMainMenuDataAsset::Get()
{
	const UNewMainMenuHUDComponent* HUDComponent = UNewMainMenuUtils::GetHUDComponent();
	checkf(HUDComponent, TEXT("ERROR: 'HUDComponent' is not initialized yet!"));
	const UNewMainMenuDataAsset* DataAsset = HUDComponent->GetNewMainMenuDataAsset();
	checkf(DataAsset, TEXT("%s: 'DataAsset' is not set"), *FString(__FUNCTION__));
	return *DataAsset;
}
