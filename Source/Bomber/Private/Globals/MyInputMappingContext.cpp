// Copyright 2021 Yevhenii Selivanov

#include "Globals/MyInputMappingContext.h"
//---
#include "EnhancedInputModule.h"
#include "Globals/MyInputAction.h"

// Returns all input actions set in mappings
void UMyInputMappingContext::GetInputActions(TArray<UMyInputAction*>& OutInputActions) const
{
	for (const FEnhancedActionKeyMapping& MappingIt : Mappings)
	{
		const UMyInputAction* MyInputAction = Cast<UMyInputAction>(MappingIt.Action);
		if (MyInputAction)
		{
			OutInputActions.AddUnique(const_cast<UMyInputAction*>(MyInputAction));
		}
	}
}

// Returns mappings by specified input action
void UMyInputMappingContext::GetMappingsByInputAction(TArray<FEnhancedActionKeyMapping>& OutMappings, const UMyInputAction* InputAction) const
{
	for (const FEnhancedActionKeyMapping& MappingIt : Mappings)
	{
		if (MappingIt.Action == InputAction)
		{
			OutMappings.Emplace(MappingIt);
		}
	}
}

// Unmap previous key and map new one
void UMyInputMappingContext::RemapKey(const UMyInputAction* InputAction, const FKey& NewKey, const FKey& PrevKey)
{
	if (!ensureMsgf(InputAction, TEXT("ASSERT: 'InputAction' is not valid")))
	{
		return;
	}

	bool bRemapped = false;
	for (FEnhancedActionKeyMapping& MappingIt : Mappings)
	{
		if (MappingIt.Action == InputAction
		    && MappingIt.Key == PrevKey)
		{
			MappingIt.Key = NewKey;
			bRemapped = true;
			break;
		}
	}

	if (!bRemapped)
	{
		return;
	}

	UEnhancedInputLibrary::RequestRebuildControlMappingsUsingContext(this);

	SaveConfig();
}
