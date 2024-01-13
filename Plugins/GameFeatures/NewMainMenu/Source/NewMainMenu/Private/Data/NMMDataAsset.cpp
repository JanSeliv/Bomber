// Copyright (c) Yevhenii Selivanov

#include "Data/NMMDataAsset.h"
//---
#include "Bomber.h"
#include "Data/NMMSubsystem.h"
#include "DataAssets/MyInputMappingContext.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNMMDataAsset& UNMMDataAsset::Get(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UNMMDataAsset* DataAsset = UNMMSubsystem::Get(OptionalWorldContext).GetNewMainMenuDataAsset();
	checkf(DataAsset, TEXT("%s: 'DataAsset' is not set"), *FString(__FUNCTION__));
	return *DataAsset;
}

// Returns an input context by given game state
const UMyInputMappingContext* UNMMDataAsset::GetInputContext(ECurrentGameState CurrentGameState) const
{
	for (const UMyInputMappingContext* InputContextIt : InputContextsInternal)
	{
		if (!InputContextIt)
		{
			continue;
		}

		const int32 GameStatesBitmask = InputContextIt->GetChosenGameStatesBitmask();
		if (GameStatesBitmask & TO_FLAG(CurrentGameState))
		{
			return InputContextIt;
		}
	}

	return nullptr;
}

// Returns all input contexts
void UNMMDataAsset::GetAllInputContexts(TArray<const UMyInputMappingContext*>& OutInputContexts) const
{
	for (const UMyInputMappingContext* InputContext : InputContextsInternal)
	{
		OutInputContexts.Emplace(InputContext);
	}
}
