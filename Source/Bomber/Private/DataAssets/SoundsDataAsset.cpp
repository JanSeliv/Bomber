// Copyright (c) Yevhenii Selivanov

#include "DataAssets/SoundsDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"

// Returns the settings data asset
const USoundsDataAsset& USoundsDataAsset::Get()
{
	const USoundsDataAsset* SoundsDataAsset = UDataAssetsContainer::GetSoundsDataAsset();
	checkf(SoundsDataAsset, TEXT("The Sounds Data Asset is not valid"));
	return *SoundsDataAsset;
}

// Returns the music of specified level
USoundBase* USoundsDataAsset::GetLevelMusic(ELevelType LevelType) const
{
	if (const TObjectPtr<USoundBase>* FoundMusic = LevelsMusicInternal.Find(LevelType))
	{
		return *FoundMusic;
	}

	return nullptr;
}

// Returns the main menu music of specified level
USoundBase* USoundsDataAsset::GetLevelMainMenuMusic(ELevelType LevelType) const
{
	if (const TObjectPtr<USoundBase>* FoundMusic = LevelsMainMenuMusicInternal.Find(LevelType))
	{
		return *FoundMusic;
	}

	return nullptr;
}

// Return the background music by specified game state and level type
USoundBase* USoundsDataAsset::GetBackgroundMusic(ECurrentGameState CurrentGameState, ELevelType LevelType) const
{
	if (CurrentGameState == ECGS::Menu)
	{
		return GetLevelMainMenuMusic(LevelType);
	}

	return GetLevelMusic(LevelType);
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
