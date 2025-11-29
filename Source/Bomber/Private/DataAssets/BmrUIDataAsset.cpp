// Copyright (c) Yevhenii Selivanov

#include "DataAssets/UIDataAsset.h"

// Bomber
#include "Bomber.h"
#include "DataAssets/DataAssetsContainer.h"

// UE
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDataAsset)

// Returns the UI data asset
const UUIDataAsset& UUIDataAsset::Get()
{
	const UUIDataAsset* UIDataAsset = UDataAssetsContainer::GetUIDataAsset();
	checkf(UIDataAsset, TEXT("The UI Data Asset is not valid"));
	return *UIDataAsset;
}

// Returns widget data associated with the given tag, or invalid widget data if not found
const FManageableWidgetData& UUIDataAsset::GetWidgetDataByTag(FGameplayTag InTag) const
{
	const FManageableWidgetData* FoundWidgetData = AllWidgetData.FindByKey(InTag);
	return FoundWidgetData ? *FoundWidgetData : FManageableWidgetData::Empty;
}

// Returns widget data associated with the given widget class, or null if not found
const FManageableWidgetData& UUIDataAsset::GetWidgetDataByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	const FManageableWidgetData* FoundWidgetData = AllWidgetData.FindByPredicate([WidgetClass](const FManageableWidgetData& It)
	{
		return It.WidgetClass == WidgetClass;
	});
	return FoundWidgetData ? *FoundWidgetData : FManageableWidgetData::Empty;
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

// Returns the default avatar for the specified player type
UTexture2D* UUIDataAsset::GetDefaultAvatar(EPlayerType PlayerType) const
{
	if (PlayerType == EPlayerType::None)
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* FoundTexturePtr = DefaultAvatarsInternal.Find(PlayerType);
	return FoundTexturePtr ? *FoundTexturePtr : nullptr;
}

// Returns the icon for the specified powerup type to display in the UI
class UTexture2D* UUIDataAsset::GetPowerupIcon(FBmrPowerupTag PowerupTag) const
{
	if (!PowerupTag.IsValid())
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* FoundTexturePtr = PowerupIconsInternal.Find(PowerupTag);
	return FoundTexturePtr ? *FoundTexturePtr : nullptr;
}