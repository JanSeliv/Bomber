// Copyright (c) Yevhenii Selivanov

#include "DataAssets/MyInputAction.h"
//---
#include "EnhancedInputSubsystems.h"
//---
#include "Controllers/MyPlayerController.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyInputAction)

// Returns the keys mapped to this action in the active input mapping contexts
void UMyInputAction::GetKeys(TArray<FKey>& OutKeys) const
{
	const AMyPlayerController* PC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = PC ? PC->GetEnhancedInputSubsystem() : nullptr;
	if (!EnhancedInputSubsystem)
	{
		return;
	}

	OutKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(this);
}

// Returns the first mapped key to this action in most priority active input context
FKey UMyInputAction::GetKey() const
{
	TArray<FKey> OutKeys;
	GetKeys(OutKeys);

	constexpr int32 KeyIndex = 0;
	static const FKey EmptyKey{};
	return OutKeys.IsValidIndex(KeyIndex) ? OutKeys[KeyIndex] : EmptyKey;
}
