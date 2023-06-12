// Copyright (c) Yevhenii Selivanov

#include "FootTrailsUtils.h"
//---
#include "GeneratedMap.h"
#include "FootTrailsGeneratorComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(FootTrailsUtils)

// Returns the Foot Trails Generator component that is responsible for generating foot trails
const UFootTrailsGeneratorComponent* UFootTrailsUtils::GetFootTrailsGeneratorComponent()
{
	return AGeneratedMap::Get().FindComponentByClass<UFootTrailsGeneratorComponent>();
}

// Returns the data asset that contains all the assets and tweaks of Foot Trails game feature
const UFootTrailsDataAsset* UFootTrailsUtils::GetFootTrailsDataAsset()
{
	const UFootTrailsGeneratorComponent* FootTrailsComponent = GetFootTrailsGeneratorComponent();
	return FootTrailsComponent ? FootTrailsComponent->GetFootTrailsDataAsset() : nullptr;
}
