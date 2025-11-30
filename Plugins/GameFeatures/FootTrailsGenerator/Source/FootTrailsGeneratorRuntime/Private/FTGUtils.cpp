// Copyright (c) Yevhenii Selivanov

#include "FTGUtils.h"

// FTG
#include "FTGComponent.h"
#include "FTGDataAsset.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FTGUtils)

// Returns the Foot Trails Generator component that is responsible for generating foot trails
const UFTGComponent* UFTGUtils::GetFootTrailsGeneratorComponent()
{
	return ABmrGeneratedMap::Get().FindComponentByClass<UFTGComponent>();
}

// Returns the data asset that contains all the assets and tweaks of Foot Trails game feature
const UFTGDataAsset* UFTGUtils::GetFootTrailsDataAsset()
{
	const UFTGComponent* FootTrailsComponent = GetFootTrailsGeneratorComponent();
	return FootTrailsComponent ? FootTrailsComponent->GetFootTrailsDataAsset() : nullptr;
}
