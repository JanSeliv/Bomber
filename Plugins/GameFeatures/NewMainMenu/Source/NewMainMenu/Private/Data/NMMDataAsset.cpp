// Copyright (c) Yevhenii Selivanov

#include "Data/NMMDataAsset.h"

// Bomber
#include "DalSubsystem.h"
#include "DataAssets/BmrInputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNMMDataAsset& UNMMDataAsset::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Returns all input contexts matching given Main Menu State
void UNMMDataAsset::GetInputContexts(FNmmStateTag MenuState, TArray<UBmrInputMappingContext*>& OutInputContexts) const
{
	for (const TTuple<TObjectPtr<UBmrInputMappingContext>, FGameplayTagContainer>& It : InputContexts)
	{
		if (It.Value.HasTag(MenuState))
		{
			OutInputContexts.AddUnique(It.Key);
		}
	}
}

// Returns all input contexts
void UNMMDataAsset::GetAllInputContexts(TArray<UBmrInputMappingContext*>& OutInputContexts) const
{
	for (const TTuple<TObjectPtr<UBmrInputMappingContext>, FGameplayTagContainer>& It : InputContexts)
	{
		OutInputContexts.Emplace(It.Key);
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
