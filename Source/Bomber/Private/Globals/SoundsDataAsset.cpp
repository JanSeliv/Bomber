// Copyright (c) Yevhenii Selivanov

#include "Globals/SoundsDataAsset.h"
//---
#include "SoundsManager.h"
#include "Globals/DataAssetsContainer.h"
#include "UtilityLibraries/SingletonLibrary.h"

// Returns the settings data asset
const USoundsDataAsset& USoundsDataAsset::Get()
{
	const USoundsDataAsset* SoundsDataAsset = UDataAssetsContainer::GetSoundsDataAsset();
	checkf(SoundsDataAsset, TEXT("The Sounds Data Asset is not valid"));
	return *SoundsDataAsset;
}

// Returns the sound manager
USoundsManager* USoundsDataAsset::GetSoundsManager() const
{
	if (!SoundsManager)
	{
		UWorld* World = USingletonLibrary::Get().GetWorld();
		SoundsManager = NewObject<USoundsManager>(World, SoundsManagerClass, NAME_None, RF_Public | RF_Transactional);
	}

	return SoundsManager;
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

// Return the background music by specified game state and level type
USoundBase* USoundsDataAsset::GetBackgroundMusic(ECurrentGameState CurrentGameState, ELevelType LevelType) const
{
	if (CurrentGameState == ECGS::Menu)
	{
		return GetMainMenuMusic();
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