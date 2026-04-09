// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrSoundsDataAsset.h"

// Bomber
#include "Bomber.h"
#include "DalSubsystem.h"
#include "DataRegistries/BmrSoundsBackgroundRow.h"

// UE
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrSoundsDataAsset)

// Returns the settings data asset
const UBmrSoundsDataAsset& UBmrSoundsDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Returns the music of specified level
USoundBase* UBmrSoundsDataAsset::GetInGameMusic(EBmrLevelType LevelType) const
{
	const FBmrSoundsBackgroundRow* Row = FBmrSoundsBackgroundRow::GetRowByPredicate([LevelType](const FBmrSoundsBackgroundRow& RowIt)
	{
		return RowIt.LevelType == LevelType || RowIt.LevelType == ELT::Max;
	});

	return Row ? Row->Music.Get() : nullptr;
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
