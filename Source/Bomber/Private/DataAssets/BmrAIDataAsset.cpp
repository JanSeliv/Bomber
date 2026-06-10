// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrAIDataAsset.h"

// Bomber
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrAIDataAsset)

// Returns the AI data asset
const UBmrAIDataAsset& UBmrAIDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Returns decision-delay multiplier for difficulty, 1 keeps full reaction when unlisted
float UBmrAIDataAsset::GetReactionSlowdown(FBmrGameDifficultyTag DifficultyTag) const
{
	constexpr float DefaultMultiplier = 1.f;
	const float* FoundMultiplier = ReactionSlowdown.Find(DifficultyTag);
	return FoundMultiplier ? FMath::Max(*FoundMultiplier, DefaultMultiplier) : DefaultMultiplier;
}
