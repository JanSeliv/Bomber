// Copyright (c) Yevhenii Selivanov

#include "DataAssets/UIDataAsset.h"
//---
#include "Bomber.h"
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDataAsset)

// Returns the UI data asset
const UUIDataAsset& UUIDataAsset::Get()
{
	const UUIDataAsset* UIDataAsset = UDataAssetsContainer::GetUIDataAsset();
	checkf(UIDataAsset, TEXT("The UI Data Asset is not valid"));
	return *UIDataAsset;
}

// Returns the localized texts about specified end game to display on UI.
const FText& UUIDataAsset::GetEndGameText(EEndGameState EndGameState) const
{
	if (EndGameState == EEndGameState::None)
	{
		return FText::GetEmpty();
	}

	return EndGameTextsInternal.FindChecked(EndGameState);
}