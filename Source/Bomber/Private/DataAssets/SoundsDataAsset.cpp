// Copyright (c) Yevhenii Selivanov

#include "DataAssets/SoundsDataAsset.h"
//---
#include "Bomber.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SoundsDataAsset)

// Returns the settings data asset
const USoundsDataAsset& USoundsDataAsset::Get()
{
	const USoundsDataAsset* SoundsDataAsset = UDataAssetsContainer::GetSoundsDataAsset();
	checkf(SoundsDataAsset, TEXT("The Sounds Data Asset is not valid"));
	return *SoundsDataAsset;
}

// Returns the music of specified level
USoundBase* USoundsDataAsset::GetInGameMusic(ELevelType LevelType) const
{
	if (const TObjectPtr<USoundBase>* FoundMusic = InGameMusicInternal.Find(LevelType))
	{
		return *FoundMusic;
	}

	return nullptr;
}

// Returns the End-Game sound by specified End-Game state
USoundBase* USoundsDataAsset::GetEndGameSFX(EEndGameState EndGameState) const
{
	if (const TObjectPtr<USoundBase>* FoundSFX = EndGameSFXInternal.Find(EndGameState))
	{
		return *FoundSFX;
	}

	return nullptr;
}
