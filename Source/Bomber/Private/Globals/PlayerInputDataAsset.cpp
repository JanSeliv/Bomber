// Copyright (c) Yevhenii Selivanov

#include "Globals/PlayerInputDataAsset.h"
//---
#include "EnhancedActionKeyMapping.h"
//---
#include "Globals/DataAssetsContainer.h"
#include "Globals/MyInputMappingContext.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#endif

// Returns the player input data asset
const UPlayerInputDataAsset& UPlayerInputDataAsset::Get()
{
	const UPlayerInputDataAsset* PlayerInputDataAsset = UDataAssetsContainer::GetPlayerInputDataAsset();
	checkf(PlayerInputDataAsset, TEXT("The Player Input Data Asset is not valid"))
	return *PlayerInputDataAsset;
}

// Returns all input contexts contained in this data asset
void UPlayerInputDataAsset::GetAllInputContexts(TArray<const UMyInputMappingContext*>& OutInputContexts) const
{
	GetAllGameplayInputContexts(OutInputContexts);

	if (const UMyInputMappingContext* MainMenuInputContext = GetMainMenuInputContext())
	{
		OutInputContexts.Emplace(MainMenuInputContext);
	}

	if (const UMyInputMappingContext* InGameMenuInputContext = GetInGameMenuInputContext())
	{
		OutInputContexts.Emplace(InGameMenuInputContext);
	}

	if (const UMyInputMappingContext* SettingsInputContext = GetSettingsInputContext())
	{
		OutInputContexts.Emplace(SettingsInputContext);
	}
}

void UPlayerInputDataAsset::GetAllGameplayInputContexts(TArray<const UMyInputMappingContext*>& OutGameplayInputContexts) const
{
	for (const UMyInputMappingContext* GameplayContextIt : GameplayInputContextsInternal)
	{
		if (GameplayContextIt)
		{
			OutGameplayInputContexts.Emplace(GameplayContextIt);
		}
	}
}

// Returns the Enhanced Input Mapping Context of gameplay actions for specified local player
const UMyInputMappingContext* UPlayerInputDataAsset::GetGameplayInputContext(int32 LocalPlayerIndex) const
{
	return GameplayInputContextsInternal.IsValidIndex(LocalPlayerIndex) ? GameplayInputContextsInternal[LocalPlayerIndex] : nullptr;
}

// Returns true if specified key is mapped to any gameplay input context
bool UPlayerInputDataAsset::IsMappedKey(const FKey& Key) const
{
	return GameplayInputContextsInternal.ContainsByPredicate([&Key](const UMyInputMappingContext* ContextIt)
	{
		return ContextIt && ContextIt->GetMappings().ContainsByPredicate([&Key](const FEnhancedActionKeyMapping& MappingIt)
		{
			return MappingIt.Key == Key;
		});
	});
}
