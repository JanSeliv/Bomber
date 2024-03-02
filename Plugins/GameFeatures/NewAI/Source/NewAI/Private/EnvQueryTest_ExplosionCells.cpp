// Copyright (c) Yevhenii Selivanov

#include "Tests/EnvQueryTest_ExplosionCells.h"
//---
#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "AISystem.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(EnvQueryTest_ExplosionCells)

UEnvQueryTest_ExplosionCells::UEnvQueryTest_ExplosionCells()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(false);

	FloatValueMin.DefaultValue = 1.f;
	FloatValueMax.DefaultValue = 1.f;
}

// Starts the test
void UEnvQueryTest_ExplosionCells::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const FCells ExplosionCells = UCellsUtilsLibrary::GetAllExplosionCells();

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FCell ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		ItemLocation.Location.Z = 0.f;
		const bool bIsExplosion = ExplosionCells.Contains(ItemLocation);
		It.SetScore(TestPurpose, FilterType, bIsExplosion, FloatValueMin.GetValue(), FloatValueMax.GetValue());
	}
}

// Returns proper description for the test
FText UEnvQueryTest_ExplosionCells::GetDescriptionDetails() const
{
	return DescribeBoolTestParams(TEXT("ExplosionCell"));
}
