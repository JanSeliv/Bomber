// Copyright (c) Yevhenii Selivanov

#include "PasBlueprintFunctionLibrary.h"

// PAS
#include "Data/PasDataAsset.h"
#include "Data/PasTurnType.h"

// Bomber
#include "DalSubsystem.h"
#include "Data/PasNextCellOutput.h"
#include "Structures/BmrCell.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PasBlueprintFunctionLibrary)

// Returns loaded PlayAreaSurrounder data asset, null if not yet loaded
const UPasDataAsset* UPasBlueprintFunctionLibrary::GetPlayAreaSurrounderData()
{
	return UDalSubsystem::GetDataAsset<UPasDataAsset>();
}

// Computes next cell on surrounder's current side
FPasNextCellOutput UPasBlueprintFunctionLibrary::GetNextCell(const FPasCellDataOnSide& InCellData)
{
	const int32 LastCol = UBmrCellUtilsLibrary::GetLastColumnIndexOnLevel();
	const int32 LastRow = UBmrCellUtilsLibrary::GetLastRowIndexOnLevel();

	FPasNextCellOutput Out;
	Out.CellDataOnSurrounderSide = InCellData;
	FPasCellDataOnSide& Next = Out.CellDataOnSurrounderSide;

	const int32 TopBound = InCellData.Hr;
	const int32 BottomBound = LastRow - InCellData.Hl;
	const int32 LeftBound = InCellData.Vu;
	const int32 RightBound = LastCol - InCellData.Vd;

	if (TopBound > BottomBound || LeftBound > RightBound)
	{
		// Boundaries crossed: surrounder finished spiralling inward
		Out.bIsOutsideBoundaries = true;
		return Out;
	}

	switch (InCellData.TurnType)
	{
		case EPasTurnType::HorizontalRight:
			Next.Row = TopBound;
			Next.Column = InCellData.Column + 1;
			if (Next.Column > RightBound)
			{
				Next.Hr = InCellData.Hr + 1;
				Next.TurnType = EPasTurnType::VerticalDown;
				Next.Column = RightBound;
				Next.Row = TopBound + 1;
				Out.bIsLastCellOnSide = true;
			}
			break;

		case EPasTurnType::VerticalDown:
			Next.Column = RightBound;
			Next.Row = InCellData.Row + 1;
			if (Next.Row > BottomBound)
			{
				Next.Vd = InCellData.Vd + 1;
				Next.TurnType = EPasTurnType::HorizontalLeft;
				Next.Row = BottomBound;
				Next.Column = RightBound - 1;
				Out.bIsLastCellOnSide = true;
			}
			break;

		case EPasTurnType::HorizontalLeft:
			Next.Row = BottomBound;
			Next.Column = InCellData.Column - 1;
			if (Next.Column < LeftBound)
			{
				Next.Hl = InCellData.Hl + 1;
				Next.TurnType = EPasTurnType::VerticalUp;
				Next.Column = LeftBound;
				Next.Row = BottomBound - 1;
				Out.bIsLastCellOnSide = true;
			}
			break;

		case EPasTurnType::VerticalUp:
			Next.Column = LeftBound;
			Next.Row = InCellData.Row - 1;
			if (Next.Row < TopBound)
			{
				Next.Vu = InCellData.Vu + 1;
				Next.TurnType = EPasTurnType::HorizontalRight;
				// Hr unchanged in VU->HR (was bumped earlier at HR->VD), OLD TopBound already equals lap row, no +1
				Next.Row = TopBound;
				Next.Column = LeftBound + 1;
				Out.bIsLastCellOnSide = true;
			}
			break;

		case EPasTurnType::None:
		default:
			Out.bIsOutsideBoundaries = true;
			return Out;
	}

	// Termination check after transition
	if (Next.Hr + Next.Hl > LastRow || Next.Vd + Next.Vu > LastCol)
	{
		Out.bIsOutsideBoundaries = true;
	}
	return Out;
}

// Returns starting cell of requested side, or invalid cell when none exists
FBmrCell UPasBlueprintFunctionLibrary::GetFirstCellOnSide(EPasTurnType TurnType)
{
	const int32 LastCol = UBmrCellUtilsLibrary::GetLastColumnIndexOnLevel();
	const int32 LastRow = UBmrCellUtilsLibrary::GetLastRowIndexOnLevel();

	switch (TurnType)
	{
		case EPasTurnType::HorizontalRight: return UBmrCellUtilsLibrary::GetCellByPositionOnLevel(0, 0);
		case EPasTurnType::VerticalDown: return UBmrCellUtilsLibrary::GetCellByPositionOnLevel(LastCol, 0);
		case EPasTurnType::HorizontalLeft: return UBmrCellUtilsLibrary::GetCellByPositionOnLevel(LastCol, LastRow);
		case EPasTurnType::VerticalUp: return UBmrCellUtilsLibrary::GetCellByPositionOnLevel(0, LastRow);
		case EPasTurnType::None:
		default: return FBmrCell::InvalidCell;
	}
}

// Returns next Num predicted cells from InCellData, stopping at boundaries
TArray<FPasCellDataOnSide> UPasBlueprintFunctionLibrary::GetNextCells(const FPasCellDataOnSide& InCellData, int32 Num)
{
	TArray<FPasCellDataOnSide> Result;
	Result.Reserve(Num);

	FPasCellDataOnSide Cursor = InCellData;
	for (int32 Index = 0; Index < Num; ++Index)
	{
		const FPasNextCellOutput Step = GetNextCell(Cursor);
		if (Step.bIsOutsideBoundaries)
		{
			break;
		}
		Result.Emplace(Step.CellDataOnSurrounderSide);
		Cursor = Step.CellDataOnSurrounderSide;
	}
	return Result;
}

// Flattens array of FPasCellDataOnSide into underlying cell set
TSet<FBmrCell> UPasBlueprintFunctionLibrary::GetCellsFromData(const TArray<FPasCellDataOnSide>& CellsData)
{
	TSet<FBmrCell> Out;
	Out.Reserve(CellsData.Num());
	for (const FPasCellDataOnSide& Entry : CellsData)
	{
		if (const FBmrCell Cell = UBmrCellUtilsLibrary::GetCellByPositionOnLevel(Entry.Column, Entry.Row); Cell.IsValid())
		{
			Out.Add(Cell);
		}
	}
	return Out;
}

// Returns active per-step delay between wall spawns, accelerating after each completed side
float UPasBlueprintFunctionLibrary::CalcWallStepTime(int32 PassedSidesNum)
{
	const UPasDataAsset* Data = GetPlayAreaSurrounderData();
	if (!Data)
	{
		// Data asset not loaded yet, fall back to 0 so caller defers timing setup
		return 0.f;
	}
	const float Denom = (Data->WallStepTimeMultiplierForEachSide * static_cast<float>(PassedSidesNum)) + 1.f;
	return Data->WallStepTime / FMath::Max(Denom, KINDA_SMALL_NUMBER);
}

// Returns active per-flicker delay derived from per-step delay split by flicker count
float UPasBlueprintFunctionLibrary::CalcFlickerTime(int32 PassedSidesNum)
{
	const UPasDataAsset* Data = GetPlayAreaSurrounderData();
	if (!Data)
	{
		// Data asset not loaded yet, fall back to 0 so caller defers timing setup
		return 0.f;
	}
	const float Flickers = static_cast<float>(FMath::Max(Data->VisualizerData.FlickersNumber, 1));
	return CalcWallStepTime(PassedSidesNum) / Flickers;
}
