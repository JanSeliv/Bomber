// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "MyTagUtilsLibrary.generated.h"

struct FPlayerTag;
struct FBmrPowerupTag;
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To PlayerTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FPlayerTag Conv_GameplayTagToPlayerTag(FGameplayTag InGameplayTag);

	/** Converts a GameplayTag to a PlayerTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InPlayerTag", DisplayName = "To GameplayTag (PlayerTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_PlayerTagToGameplayTag(FPlayerTag InPlayerTag);

	/** Converts a PowerupTag to a GameplayTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To PowerupTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FBmrPowerupTag Conv_GameplayTagToPowerupTag(FGameplayTag InGameplayTag);

	/** Converts a GameplayTag to a PowerupTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InPowerupTag", DisplayName = "To GameplayTag (PowerupTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_PowerupTagToGameplayTag(FBmrPowerupTag InPowerupTag);
};
