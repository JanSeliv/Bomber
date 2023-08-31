// Copyright (c) Yevhenii Selivanov

#include "DataAssets/PlayerInputDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/MyInputMappingContext.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerInputDataAsset)

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
	TryCreateGameplayInputContexts();
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
	TryCreateGameplayInputContexts();
	return GameplayInputContextsInternal.IsValidIndex(LocalPlayerIndex) ? GameplayInputContextsInternal[LocalPlayerIndex] : nullptr;
}

// Returns true if specified key is mapped to any gameplay input context
bool UPlayerInputDataAsset::IsMappedKey(const FKey& Key) const
{
	return GameplayInputContextsInternal.ContainsByPredicate([&Key](const UMyInputMappingContext* ContextIt)
	{
		return ContextIt && UInputUtilsLibrary::IsMappedKeyInContext(Key, ContextIt);
	});
}

// Creates new contexts if is needed
void UPlayerInputDataAsset::TryCreateGameplayInputContexts() const
{
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		// Do not create input contexts since the game is not started yet
		return;
	}

	// Create new context if any is null
	const int32 ClassesNum = GameplayInputContextClassesInternal.Num();
	for (int32 Index = 0; Index < ClassesNum; ++Index)
	{
		const bool bIsValidIndex = GameplayInputContextsInternal.IsValidIndex(Index);
		const UMyInputMappingContext* GameplayInputContextsIt = bIsValidIndex ? GameplayInputContextsInternal[Index] : nullptr;
		if (GameplayInputContextsIt)
		{
			// Is already created
			continue;
		}

		// Initialize new gameplay contexts
		UWorld* World = UMyBlueprintFunctionLibrary::GetStaticWorld();
		const TSubclassOf<UMyInputMappingContext>& ContextClassIt = GameplayInputContextClassesInternal[Index];
		if (!World
		    || !ContextClassIt)
		{
			// Is empty class
			continue;
		}

		const FName ContextClassName(*FString::Printf(TEXT("%s_%i"), *ContextClassIt->GetName(), Index));
		UMyInputMappingContext* NewGameplayInputContext = NewObject<UMyInputMappingContext>(World, ContextClassIt, ContextClassName, RF_Public | RF_Transactional);

		if (bIsValidIndex)
		{
			GameplayInputContextsInternal[Index] = NewGameplayInputContext;
		}
		else
		{
			GameplayInputContextsInternal.EmplaceAt(Index, NewGameplayInputContext);
		}
	}
}
