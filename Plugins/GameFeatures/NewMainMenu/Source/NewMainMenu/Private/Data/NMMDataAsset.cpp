// Copyright (c) Yevhenii Selivanov

#include "Data/NMMDataAsset.h"

// NMM
#include "Subsystems/NMMBaseSubsystem.h"

// Bomber
#include "DataAssets/BmrInputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNMMDataAsset& UNMMDataAsset::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UNMMDataAsset* DataAsset = UNMMBaseSubsystem::Get(OptionalWorldContext).GetNewMainMenuDataAsset();
	checkf(DataAsset, TEXT("%s: 'DataAsset' is not set"), *FString(__FUNCTION__));
	return *DataAsset;
}

// Returns an input context by given Main Menu State
const UBmrInputMappingContext* UNMMDataAsset::GetInputContext(ENMMState MenuState) const
{
	const TObjectPtr<const UBmrInputMappingContext>* FoundContext = InputContexts.Find(MenuState);
	return FoundContext ? *FoundContext : nullptr;
}

// Returns all input contexts
void UNMMDataAsset::GetAllInputContexts(TArray<const UBmrInputMappingContext*>& OutInputContexts) const
{
	for (const TTuple<ENMMState, TObjectPtr<const UBmrInputMappingContext>>& It : InputContexts)
	{
		OutInputContexts.Emplace(It.Value);
	}
}

// Returns the main menu music of specified level
USoundBase* UNMMDataAsset::GetMainMenuMusic(EBmrLevelType LevelType) const
{
	if (const TObjectPtr<USoundBase>* FoundMusic = MainMenuMusic.Find(LevelType))
	{
		return *FoundMusic;
	}

	return nullptr;
}

// Returns all main menu music
void UNMMDataAsset::GetAllMainMenuMusic(TArray<USoundBase*>& OutMainMenuMusic) const
{
	for (const TTuple<EBmrLevelType, TObjectPtr<USoundBase>>& It : MainMenuMusic)
	{
		OutMainMenuMusic.AddUnique(It.Value);
	}
}
