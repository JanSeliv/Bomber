// Copyright (c) Yevhenii Selivanov

#include "Data/NewAIDataProviders.h"
//---
#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIDataProviders)

void UNewAIDataProvider_HalfGridSize::BindData(const UObject& Owner, int32 RequestId)
{
	Super::BindData(Owner, RequestId);

	const FIntPoint ScaleXY = UCellsUtilsLibrary::GetLevelGridScale();
	const float LargerSize = FMath::Max(ScaleXY.X, ScaleXY.Y) * UCellsUtilsLibrary::GetCellSize();
	FloatValue = LargerSize * 0.5f;
	IntValue = FMath::RoundToInt(FloatValue);
	BoolValue = IntValue != 0;
}

void UNewAIDataProvider_CellSize::BindData(const UObject& Owner, int32 RequestId)
{
	Super::BindData(Owner, RequestId);

	FloatValue = UCellsUtilsLibrary::GetCellSize();
	IntValue = FMath::RoundToInt(FloatValue);
	BoolValue = IntValue != 0;
}
