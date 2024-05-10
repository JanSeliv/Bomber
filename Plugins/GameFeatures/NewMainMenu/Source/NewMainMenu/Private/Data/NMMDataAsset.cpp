// Copyright (c) Yevhenii Selivanov

#include "Data/NMMDataAsset.h"
//---
#include "DataAssets/MyInputMappingContext.h"
#include "Subsystems/NMMBaseSubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMDataAsset)

// Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set
const UNMMDataAsset& UNMMDataAsset::Get(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UNMMDataAsset* DataAsset = UNMMBaseSubsystem::Get(OptionalWorldContext).GetNewMainMenuDataAsset();
	checkf(DataAsset, TEXT("%s: 'DataAsset' is not set"), *FString(__FUNCTION__));
	return *DataAsset;
}

// Returns an input context by given Main Menu State
const UMyInputMappingContext* UNMMDataAsset::GetInputContext(ENMMState MenuState) const
{
	const TObjectPtr<const UMyInputMappingContext>* FoundContext = InputContextsInternal.Find(MenuState);
	return FoundContext ? *FoundContext : nullptr;
}

// Returns all input contexts
void UNMMDataAsset::GetAllInputContexts(TArray<const UMyInputMappingContext*>& OutInputContexts) const
{
	for (const TTuple<ENMMState, TObjectPtr<const UMyInputMappingContext>>& It : InputContextsInternal)
	{
		OutInputContexts.Emplace(It.Value);
	}
}
