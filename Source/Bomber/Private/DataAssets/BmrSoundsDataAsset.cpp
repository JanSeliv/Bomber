// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrSoundsDataAsset.h"

// Bomber
#include "Bomber.h"
#include "DataAssets/BmrDataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrSoundsDataAsset)

// Returns the settings data asset
const UBmrSoundsDataAsset& UBmrSoundsDataAsset::Get()
{
	const UBmrSoundsDataAsset* SoundsDataAsset = UBmrDataAssetsContainer::GetSoundsDataAsset();
	checkf(SoundsDataAsset, TEXT("The Sounds Data Asset is not valid"));
	return *SoundsDataAsset;
}

// Returns the music of specified level
USoundBase* UBmrSoundsDataAsset::GetInGameMusic(EBmrLevelType LevelType) const
{
	if (const TObjectPtr<USoundBase>* FoundMusic = InGameMusic.Find(LevelType))
	{
		return *FoundMusic;
	}

	return nullptr;
}

// Returns the End-Game sound by specified End-Game state
USoundBase* UBmrSoundsDataAsset::GetEndGameSFX(EBmrEndGameState EndGameState) const
{
	if (const TObjectPtr<USoundBase>* FoundSFX = EndGameSFX.Find(EndGameState))
	{
		return *FoundSFX;
	}

	return nullptr;
}
