// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BmrTagUtilsLibrary.generated.h"

struct FBmrGameDifficultyTag;
struct FBmrPlayerTag;
struct FBmrPowerupTag;
struct FGameplayTag;
struct FBmrGameStateTag;

/**
 * Contains function for blueprint developers to work with custom tag structure like converters to FBmrPlayerTag.
 */
UCLASS()
class BOMBER_API UBmrTagUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Converts a PlayerTag to a GameplayTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To PlayerTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FBmrPlayerTag Conv_GameplayTagToPlayerTag(FGameplayTag InGameplayTag);

	/** Converts a GameplayTag to a PlayerTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InPlayerTag", DisplayName = "To GameplayTag (PlayerTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_PlayerTagToGameplayTag(FBmrPlayerTag InPlayerTag);

	/** Converts a PowerupTag to a GameplayTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To PowerupTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FBmrPowerupTag Conv_GameplayTagToPowerupTag(FGameplayTag InGameplayTag);

	/** Converts a GameplayTag to a PowerupTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InPowerupTag", DisplayName = "To GameplayTag (PowerupTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_PowerupTagToGameplayTag(FBmrPowerupTag InPowerupTag);

	/** Converts a GameStateTag to a GameplayTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To GameStateTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FBmrGameStateTag Conv_GameplayTagToGameStateTag(FGameplayTag InGameplayTag);

	/** Converts a GameplayTag to a GameStateTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InGameStateTag", DisplayName = "To GameplayTag (GameStateTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_GameStateTagToGameplayTag(FBmrGameStateTag InGameStateTag);

	/** Converts a GameplayTag to a GameDifficultyTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InGameplayTag", DisplayName = "To GameDifficultyTag (GameplayTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FBmrGameDifficultyTag Conv_GameplayTagToGameDifficultyTag(FGameplayTag InGameplayTag);

	/** Converts a GameDifficultyTag to a GameplayTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InGameDifficultyTag", DisplayName = "To GameplayTag (GameDifficultyTag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayTag Conv_GameDifficultyTagToGameplayTag(FBmrGameDifficultyTag InGameDifficultyTag);
};
