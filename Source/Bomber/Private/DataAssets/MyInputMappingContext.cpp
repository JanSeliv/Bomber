// Copyright (c) Yevhenii Selivanov

#include "DataAssets/MyInputMappingContext.h"
//---
#include "DataAssets/MyInputAction.h"
#include "DataAssets/PlayerInputDataAsset.h"
//---
#include "EnhancedInputLibrary.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyInputMappingContext)

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
	TArray<FEnhancedActionKeyMapping> AllMappings;
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		if (MappingIt.Action == InputAction)
		{
			OutMappings.Emplace(MappingIt);
		}
	}
}

// Returns all mappings where bIsPlayerMappable is true
void UMyInputMappingContext::GetAllMappings(TArray<FEnhancedActionKeyMapping>& OutMappableData) const
{
	for (const FEnhancedActionKeyMapping& MappingIt : Mappings)
	{
		if (MappingIt.IsPlayerMappable())
		{
			OutMappableData.Emplace(MappingIt);
		}
	}
}

// Unmap previous key and map new one
bool UMyInputMappingContext::RemapKey(const UInputAction* InInputAction, const FKey& NewKey, const FKey& PrevKey)
{
	if (!ensureMsgf(InInputAction, TEXT("ASSERT: 'InInputAction' is not valid"))
	    || NewKey == PrevKey
	    || UPlayerInputDataAsset::Get().IsMappedKey(NewKey))
	{
		return false;
	}

	bool bRemapped = false;
	for (FEnhancedActionKeyMapping& MappingIt : Mappings)
	{
		if (MappingIt.Action == InInputAction
		    && MappingIt.Key == PrevKey)
		{
			MappingIt.Key = NewKey;
			bRemapped = true;
			break;
		}
	}

	if (!bRemapped)
	{
		return false;
	}

	UEnhancedInputLibrary::RequestRebuildControlMappingsUsingContext(this);

	if (CanSaveMappingsInConfig())
	{
		SaveConfig();
	}

	return true;
}

// Unmap previous key and map new one
bool UMyInputMappingContext::RemapKey(const UMyInputMappingContext* InContext, const FEnhancedActionKeyMapping& InMapping, const FKey& NewKey)
{
	if (!ensureMsgf(InContext, TEXT("%s: 'InContext' is not valid"), *FString(__FUNCTION__)))
	{
		return false;
	}

	UMyInputMappingContext* ContextToRemap = const_cast<UMyInputMappingContext*>(InContext);
	checkf(ContextToRemap, TEXT("%s: 'ContextToRemap' is null"), *FString(__FUNCTION__));
	return ContextToRemap->RemapKey(InMapping.Action, NewKey, InMapping.Key);
}

// Returns true if remapped key is allowed to be saved in config
bool UMyInputMappingContext::CanSaveMappingsInConfig()
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	// We don't want to save remaps in Editor, it gets serialised right into asset
	return !FEditorUtilsLibrary::IsEditor();
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	// Always return true in cook since there remaps should be saved into config file and taken there.
	return true;
}
