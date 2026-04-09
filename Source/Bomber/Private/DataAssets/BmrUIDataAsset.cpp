// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrUIDataAsset.h"

// Bomber
#include "Bomber.h"
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrUIDataAsset)

// Returns the UI data asset
const UBmrUIDataAsset& UBmrUIDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Returns the localized texts about specified end game to display on UI.
const FText& UBmrUIDataAsset::GetEndGameText(EBmrEndGameState EndGameState) const
{
	if (EndGameState == EBmrEndGameState::None)
	{
		return FText::GetEmpty();
	}

	return EndGameTexts.FindChecked(EndGameState);
}

// Returns the default avatar for the specified player type
UTexture2D* UBmrUIDataAsset::GetDefaultAvatar(EBmrPlayerType PlayerType) const
{
	if (PlayerType == EBmrPlayerType::None)
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* FoundTexturePtr = DefaultAvatars.Find(PlayerType);
	return FoundTexturePtr ? *FoundTexturePtr : nullptr;
}

// Returns the icon for the specified powerup type to display in the UI
class UTexture2D* UBmrUIDataAsset::GetPowerupIcon(FBmrPowerupTag PowerupTag) const
{
	if (!PowerupTag.IsValid())
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* FoundTexturePtr = PowerupIcons.Find(PowerupTag);
	return FoundTexturePtr ? *FoundTexturePtr : nullptr;
}