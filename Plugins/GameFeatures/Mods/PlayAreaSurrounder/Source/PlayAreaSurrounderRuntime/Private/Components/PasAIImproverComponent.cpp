// Copyright (c) Yevhenii Selivanov

#include "Components/PasAIImproverComponent.h"

// PAS
#include "Data/PasCellDataOnSide.h"
#include "Data/PasDataAsset.h"
#include "PasBlueprintFunctionLibrary.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Structures/BmrCell.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PasAIImproverComponent)

UPasAIImproverComponent::UPasAIImproverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// When surrounder queries how many cells ahead to predict for this component
int32 UPasAIImproverComponent::GetPredictionNum_Implementation() const
{
	const UPasDataAsset* Data = UPasBlueprintFunctionLibrary::GetPlayAreaSurrounderData();
	return Data ? FMath::Max(Data->UnsafeCellsNumInFront, 1) : 1;
}

// When surrounder emits next predicted cells after each wall step
void UPasAIImproverComponent::HandlePrediction_Implementation(const TArray<FPasCellDataOnSide>& PredictedCells)
{
	Super::HandlePrediction_Implementation(PredictedCells);

	DangerousCells.Reset();
	DangerousCells.Reserve(PredictedCells.Num());
	for (const FPasCellDataOnSide& Entry : PredictedCells)
	{
		if (const FBmrCell Cell = UBmrCellUtilsLibrary::GetCellByPositionOnLevel(Entry.Column, Entry.Row); Cell.IsValid())
		{
			DangerousCells.Add(Cell);
		}
	}

	// Union prediction into BmrGeneratedMap's
	// AdditionalDangerousCells so game AI / bot logic reading from
	// map (not from this component) sees surrounder-ahead cells
	if (GeneratedMap)
	{
		GeneratedMap->AdditionalDangerousCells = GeneratedMap->AdditionalDangerousCells.Union(DangerousCells);
	}
}
