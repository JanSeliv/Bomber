// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrUIDataAsset.h"

// Bomber
#include "Bomber.h"
#include "DalSubsystem.h"

// UE
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrUIDataAsset)

// Returns the UI data asset
const UBmrUIDataAsset& UBmrUIDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Returns widget data associated with the given tag, or invalid widget data if not found
const FBmrManageableWidgetData& UBmrUIDataAsset::GetWidgetDataByTag(FGameplayTag InTag) const
{
	const FBmrManageableWidgetData* FoundWidgetData = AllWidgetData.FindByKey(InTag);
	return FoundWidgetData ? *FoundWidgetData : FBmrManageableWidgetData::Empty;
}

// Returns widget data associated with the given widget class, or null if not found
const FBmrManageableWidgetData& UBmrUIDataAsset::GetWidgetDataByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	const FBmrManageableWidgetData* FoundWidgetData = AllWidgetData.FindByPredicate([WidgetClass](const FBmrManageableWidgetData& It)
	{
		return It.WidgetClass == WidgetClass;
	});
	return FoundWidgetData ? *FoundWidgetData : FBmrManageableWidgetData::Empty;
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