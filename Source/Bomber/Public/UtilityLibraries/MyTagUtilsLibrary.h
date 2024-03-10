// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "MyTagUtilsLibrary.generated.h"

struct FPlayerTag;
struct FGameplayTag;

/**
 * Contains function for blueprint developers to work with custom tag structure like converters to FPlayerTag.
 */
UCLASS()
class BOMBER_API UMyTagUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Converts a PlayerTag to a GameplayTag. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To PlayerTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FPlayerTag Conv_GameplayTagToPlayerTag(FGameplayTag InGameplayTag);

	/** Converts a GameplayTag to a PlayerTag. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InPlayerTag", DisplayName = "To GameplayTag (PlayerTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_PlayerTagToGameplayTag(FPlayerTag InPlayerTag);
};
