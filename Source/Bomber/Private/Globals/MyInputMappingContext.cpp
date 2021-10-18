// Copyright 2021 Yevhenii Selivanov

#include "Globals/MyInputMappingContext.h"
//---
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
