﻿// Copyright (c) Yevhenii Selivanov

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
		return ContextIt && ContextIt->GetMappings().ContainsByPredicate([&Key](const FEnhancedActionKeyMapping& MappingIt)
		{
			return MappingIt.Key == Key;
		});
	});
}

// Creates new contexts if is needed
void UPlayerInputDataAsset::TryCreateGameplayInputContexts() const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		// Do not create input contexts since the game is not started yet
		return;
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

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
		UWorld* World = USingletonLibrary::Get().GetWorld();
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