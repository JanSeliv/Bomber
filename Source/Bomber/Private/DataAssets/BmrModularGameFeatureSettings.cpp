// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrModularGameFeatureSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrModularGameFeatureSettings)

// Returns aggregated container of all unique tags from tag-driven features map, lazy-cached
const FGameplayTagContainer& UBmrModularGameFeatureSettings::GetAllModularGameFeatureTags() const
{
	static FGameplayTagContainer CachedTags;
	if (!CachedTags.IsEmpty())
	{
		return CachedTags;
	}

	for (const TTuple<FName, FGameplayTagContainer>& Pair : ModularGameFeaturesByTags)
	{
		CachedTags.AppendTags(Pair.Value);
	}

	return CachedTags;
}