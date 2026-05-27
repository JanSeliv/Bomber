// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrSoundsDataAsset.h"

// Bomber
#include "Bomber.h"
#include "DalSubsystem.h"

// UE
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrSoundsDataAsset)

// Returns the settings data asset
const UBmrSoundsDataAsset& UBmrSoundsDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
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
