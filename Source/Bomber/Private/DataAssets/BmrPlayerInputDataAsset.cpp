// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrPlayerInputDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"
#include "DataAssets/BmrInputMappingContext.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerInputDataAsset)

// Returns the player input data asset
const UBmrPlayerInputDataAsset& UBmrPlayerInputDataAsset::Get()
{
	const UBmrPlayerInputDataAsset* PlayerInputDataAsset = UBmrDataAssetsContainer::GetPlayerInputDataAsset();
	checkf(PlayerInputDataAsset, TEXT("The Player Input Data Asset is not valid")) return *PlayerInputDataAsset;
}

// Returns all input contexts contained in this data asset
void UBmrPlayerInputDataAsset::GetAllInputContexts(TArray<const UBmrInputMappingContext*>& OutInputContexts) const
{
	GetAllGameplayInputContexts(OutInputContexts);

	if (InGameMenuInputContext)
	{
		OutInputContexts.Emplace(InGameMenuInputContext);
	}

	if (SettingsInputContext)
	{
		OutInputContexts.Emplace(SettingsInputContext);
	}
}

void UBmrPlayerInputDataAsset::GetAllGameplayInputContexts(TArray<const UBmrInputMappingContext*>& OutGameplayInputContexts) const
{
	TryCreateGameplayInputContexts();
	for (const UBmrInputMappingContext* GameplayContextIt : GameplayInputContexts)
	{
		if (GameplayContextIt)
		{
			OutGameplayInputContexts.Emplace(GameplayContextIt);
		}
	}
}

// Returns the Enhanced Input Mapping Context of gameplay actions for specified local player
const UBmrInputMappingContext* UBmrPlayerInputDataAsset::GetGameplayInputContext(int32 LocalPlayerIndex) const
{
	TryCreateGameplayInputContexts();
	return GameplayInputContexts.IsValidIndex(LocalPlayerIndex) ? GameplayInputContexts[LocalPlayerIndex] : nullptr;
}

// Returns the mouse visibility settings by specified game state tag
const FBmrMouseVisibilitySettings& UBmrPlayerInputDataAsset::GetMouseVisibilitySettings(FBmrGameStateTag GameStateTag) const
{
	const FBmrMouseVisibilitySettings* FoundSettingsPtr = MouseVisibilitySettings.FindByKey(GameStateTag);
	return FoundSettingsPtr ? *FoundSettingsPtr : FBmrMouseVisibilitySettings::Invalid;
}

// Returns the mouse visibility settings by custom game state
const FBmrMouseVisibilitySettings& UBmrPlayerInputDataAsset::GetMouseVisibilitySettingsCustom(FName CustomGameState) const
{
	const FBmrMouseVisibilitySettings* FoundSettingsPtr = MouseVisibilitySettings.FindByKey(CustomGameState);
	return FoundSettingsPtr ? *FoundSettingsPtr : FBmrMouseVisibilitySettings::Invalid;
}

// Returns true if specified key is mapped to any gameplay input context
bool UBmrPlayerInputDataAsset::IsMappedKey(const UObject* WorldContext, const FKey& Key) const
{
	return GameplayInputContexts.ContainsByPredicate([&Key, WorldContext](const UBmrInputMappingContext* ContextIt)
	{
		return ContextIt && UInputUtilsLibrary::IsMappedKeyInContext(WorldContext, Key, ContextIt);
	});
}

// Creates new contexts if is needed
void UBmrPlayerInputDataAsset::TryCreateGameplayInputContexts() const
{
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		// Do not create input contexts since the game is not started yet
		return;
	}

	// Create new context if any is null
	const int32 ClassesNum = GameplayInputContextClasses.Num();
	for (int32 Index = 0; Index < ClassesNum; ++Index)
	{
		const bool bIsValidIndex = GameplayInputContexts.IsValidIndex(Index);
		const UBmrInputMappingContext* GameplayInputContextsIt = bIsValidIndex ? GameplayInputContexts[Index] : nullptr;
		if (GameplayInputContextsIt)
		{
			// Is already created
			continue;
		}

		// Initialize new gameplay contexts
		UWorld* World = UUtilsLibrary::GetPlayWorld();
		const TSubclassOf<UBmrInputMappingContext>& ContextClassIt = GameplayInputContextClasses[Index];
		if (!World
		    || !ContextClassIt)
		{
			// Is empty class
			continue;
		}

		const FName ContextClassName(*FString::Printf(TEXT("%s_%i"), *ContextClassIt->GetName(), Index));
		UBmrInputMappingContext* NewGameplayInputContext = NewObject<UBmrInputMappingContext>(World, ContextClassIt, ContextClassName, RF_Public | RF_Transactional);

		if (bIsValidIndex)
		{
			GameplayInputContexts[Index] = NewGameplayInputContext;
		}
		else
		{
			GameplayInputContexts.EmplaceAt(Index, NewGameplayInputContext);
		}
	}
}