// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/TextRenderComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(CellsUtilsLibrary)

// Default params to display cells
const FDisplayCellsParams FDisplayCellsParams::EmptyParams = FDisplayCellsParams();

// Creates 'Break Cell' node with X, Y, Z outputs
void UCellsUtilsLibrary::BreakCell(const FCell& InCell, double& X, double& Y, double& Z)
{
	X = InCell.X();
	Y = InCell.Y();
	Z = InCell.Z();
}

// ---------------------------------------------------
//		Grid transform library
// ---------------------------------------------------

// Returns transform of cells grid on current level
FTransform UCellsUtilsLibrary::GetLevelGridTransform()
{
	// As alternative, GeneratedMap's transform could be taken, but array transform is more reliable
	return GetCellArrayTransform(GetAllCellsOnLevel());
}

// Returns location of grid pivot on current level
FVector UCellsUtilsLibrary::GetLevelGridLocation()
{
	// As alternative #1, GeneratedMap's location could be taken, but cell array location is more reliable
	// As alternative #2, GetCellArrayCenter could be used, but it's more expensive
	return GetCenterCellOnLevel().Location;
}

// Returns any cell Z-location on the Generated Map
float UCellsUtilsLibrary::GetCellHeightLocation()
{
	return GetLevelGridLocation().Z;
}

// Returns cell rotator that is the same for any cell on the Generated Map
FRotator UCellsUtilsLibrary::GetLevelGridRotation()
{
	// As alternative, GeneratedMap's rotation could be taken, but cell array rotation is more reliable
	return GetCellArrayRotation(GetAllCellsOnLevel());
}

// Returns cell yaw angle in degrees that is the same for any cell on the Generated Map
float UCellsUtilsLibrary::GetCellYawDegree()
{
	return GetLevelGridRotation().Yaw;
}

// Returns the current grid size
FIntPoint UCellsUtilsLibrary::GetLevelGridScale()
{
	// As alternative, GeneratedMap's scale could be taken, but cell array size is more reliable
	const FVector2D GridSize = GetCellArraySize(GetAllCellsOnLevel());
	return FIntPoint(GridSize.X, GridSize.Y);
}

// Returns the width (number of columns X) of the Generated Map
int32 UCellsUtilsLibrary::GetCellColumnsNumOnLevel()
{
	return FMath::FloorToInt32(GetCellArrayWidth(GetAllCellsOnLevel()));
}

// Returns the length (number of rows Y) of the Generated Map
int32 UCellsUtilsLibrary::GetCellRowsNumOnLevel()
{
	return FMath::FloorToInt32(GetCellArrayLength(GetAllCellsOnLevel()));
}

// Returns GetCellColumnsNumOnLevel - 1
int32 UCellsUtilsLibrary::GetLastColumnIndexOnLevel()
{
	return GetCellColumnsNumOnLevel() - 1;
}

// Returns Returns GetCellRowsNumOnLevel - 1
int32 UCellsUtilsLibrary::GetLastRowIndexOnLevel()
{
	return GetCellRowsNumOnLevel() - 1;
}

// ---------------------------------------------------
//		Generated Map related cell functions
// ---------------------------------------------------

// Takes the cell and returns its row and column position on the level if exists, -1 otherwise
void UCellsUtilsLibrary::GetPositionByCellOnLevel(const FCell& InCell, int32& OutColumnX, int32& OutRowY)
{
	const FIntPoint CellPosition = GetPositionByCellOnGrid(InCell, GetAllCellsOnLevel());
	OutColumnX = CellPosition.X;
	OutRowY = CellPosition.Y;
}

// Returns all grid cell location on the Generated Map
const TArray<FCell>& UCellsUtilsLibrary::GetAllCellsOnLevelAsArray()
{
	return AGeneratedMap::Get().GridCellsInternal;
}

// Returns the cell location of the Generated Map
FCell UCellsUtilsLibrary::GetCenterCellOnLevel()
{
	int32 CenterColumn = INDEX_NONE;
	int32 CenterRow = INDEX_NONE;
	GetCenterCellPositionOnLevel(/*out*/CenterRow, /*out*/CenterColumn);
	return GetCellByPositionOnLevel(CenterColumn, CenterRow);
}

// Returns the center row and column positions on the level
void UCellsUtilsLibrary::GetCenterCellPositionOnLevel(int32& OutColumnX, int32& OutRowY)
{
	const FIntPoint CenterCellPosition = GetCenterCellPositionOnGrid(GetAllCellsOnLevel());
	OutColumnX = CenterCellPosition.X;
	OutRowY = CenterCellPosition.Y;
}

// Return closest corner cell to the given cell
FCell UCellsUtilsLibrary::GetNearestCornerCellOnLevel(const FCell& CellToCheck)
{
	const TSet<FCell> AllCornerCells = GetCornerCellsOnLevel();
	return GetCellArrayNearest(AllCornerCells, CellToCheck);
}

// Returns all empty grid cell locations on the Generated Map where non of actors are present
FCells UCellsUtilsLibrary::GetAllEmptyCellsWithoutActors()
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetAllCellsWithActors(NoneActorType);
}

// Returns all grid cell location on the Generated Map by specified actor types
FCells UCellsUtilsLibrary::GetAllCellsWithActors(int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = true;
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return OutCells;
}

// Takes cells and returns only empty cells where non of actors are present
FCells UCellsUtilsLibrary::FilterEmptyCellsWithoutActors(const FCells& InCells)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return FilterCellsByActors(InCells, NoneActorType);
}

// Takes cells and returns only matching with specified actor types
FCells UCellsUtilsLibrary::FilterCellsByActors(const FCells& InCells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells OutCells = InCells;
	AGeneratedMap::Get().IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return OutCells;
}

// Returns true if cell is empty, so it does not have own actor
bool UCellsUtilsLibrary::IsEmptyCellWithoutActor(const FCell& Cell)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return IsCellHasAnyMatchingActor(Cell, NoneActorType);
}

// Checking the containing of the specified cell among owners locations of the Map Components array
bool UCellsUtilsLibrary::IsCellHasAnyMatchingActor(const FCell& Cell, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cell};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Contains(Cell);
}

// Returns true if at least one cell is empty, so it does not have own actor
bool UCellsUtilsLibrary::IsAnyCellEmptyWithoutActor(const FCells& Cells)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return AreCellsHaveAnyMatchingActors(Cells, NoneActorType);
}

// Returns true if at least one cell has actors of specified types
bool UCellsUtilsLibrary::AreCellsHaveAnyMatchingActors(const FCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cells};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return !NonEmptyCells.IsEmpty();
}

// Returns true if all cells are empty, so don't have own actors
bool UCellsUtilsLibrary::AreAllCellsEmptyWithoutActors(const FCells& Cells)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return AreCellsHaveAllMatchingActors(Cells, NoneActorType);
}

// Returns true if all cells have actors of specified types
bool UCellsUtilsLibrary::AreCellsHaveAllMatchingActors(const FCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cells};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Num() == Cells.Num();
}

// Returns cells around the center in specified radius and according desired type of breaks
FCells UCellsUtilsLibrary::GetCellsAround(const FCell& CenterCell, EPathType Pathfinder, int32 Radius)
{
	constexpr int32 AllDirections = TO_FLAG(ECellDirection::All);
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, Radius, AllDirections);
	return OutCells;
}

// Returns cells that match specified actors in specified radius from a center, according desired type of breaks
FCells UCellsUtilsLibrary::GetCellsAroundWithActors(const FCell& CenterCell, EPathType Pathfinder, int32 Radius, int32 ActorsTypesBitmask)
{
	ensureMsgf(Pathfinder == EPathType::Any || !EnumHasAnyFlags(EAT::Wall, TO_ENUM(EAT, ActorsTypesBitmask)), TEXT("ASSERT: Is trying to find walls for a pathfinder that breaks a path by walls"));
	const FCells CellsAround = GetCellsAround(CenterCell, Pathfinder, Radius);
	return FilterCellsByActors(CellsAround, ActorsTypesBitmask);
}

// Returns matching empty cells around without actors, according desired type of breaks
FCells UCellsUtilsLibrary::GetEmptyCellsAroundWithoutActors(const FCell& CenterCell, EPathType Pathfinder, int32 Radius)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetCellsAroundWithActors(CenterCell, Pathfinder, Radius, NoneActorType);
}

// Returns first cell in specified direction from a center
FCell UCellsUtilsLibrary::GetCellInDirection(const FCell& CenterCell, EPathType Pathfinder, ECellDirection Direction)
{
	ensureMsgf(Direction != ECellDirection::All, TEXT("ASSERT: Is specified 'ECellDirection::All' while function could return only cell in 1 direction"));
	constexpr int32 SideLength = 1;
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, SideLength, TO_FLAG(Direction));
	OutCells.Remove(CenterCell);
	return FCell::GetFirstCellInSet(OutCells);
}

// Returns true if a cell was found in specified direction from a center, according desired type of breaks
bool UCellsUtilsLibrary::CanGetCellInDirection(const FCell& CenterCell, EPathType Pathfinder, ECellDirection Direction)
{
	return GetCellInDirection(CenterCell, Pathfinder, Direction).IsValid();
}

// Returns cells in specified direction from a center
FCells UCellsUtilsLibrary::GetCellsInDirections(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask)
{
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, SideLength, DirectionsBitmask);
	return OutCells;
}

// Returns cells that match specified actors in specified direction from a center
FCells UCellsUtilsLibrary::GetCellsInDirectionsWithActors(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask, int32 ActorsTypesBitmask)
{
	ensureMsgf(Pathfinder == EPathType::Any || !EnumHasAnyFlags(EAT::Wall, TO_ENUM(EAT, ActorsTypesBitmask)), TEXT("ASSERT: Is trying to find walls for a pathfinder that breaks a path by walls"));
	const FCells CellsInDirections = GetCellsInDirections(CenterCell, Pathfinder, SideLength, DirectionsBitmask);
	return FilterCellsByActors(CellsInDirections, ActorsTypesBitmask);
}

// Returns matching empty cells without actors in chosen direction(s), according desired type of breaks
FCells UCellsUtilsLibrary::GetEmptyCellsInDirectionsWithoutActors(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetCellsInDirectionsWithActors(CenterCell, Pathfinder, SideLength, DirectionsBitmask, NoneActorType);
}

// Returns true if player is not able to reach specified cell by any any path
bool UCellsUtilsLibrary::IsIslandCell(const FCell& Cell)
{
	return !AGeneratedMap::Get().DoesPathExistToCells({Cell});
}

// Rotates the given cell around the center of the Generated Map to the same yaw degree
FCell UCellsUtilsLibrary::RotateCellAroundLevelOrigin(const FCell& Cell, float AxisZ)
{
	const FTransform GridTransformNoScale = FCell::GetCellArrayTransformNoScale(GetAllCellsOnLevel());
	return FCell::RotateCellAroundOrigin(Cell, AxisZ, GridTransformNoScale);
}

// Returns nearest free cell to given cell, where free means cell with no other level actors except players
FCell UCellsUtilsLibrary::GetNearestFreeCell(const FCell& Cell)
{
	FCells FreeCells = GetAllEmptyCellsWithoutActors();
	FreeCells.Append(GetAllCellsWithActors(TO_FLAG(EAT::Player))); // Players are also considered as free cells
	return GetCellArrayNearest(FreeCells, Cell);
}

// ---------------------------------------------------
//		 Debug cells utilities
// ---------------------------------------------------

// Remove all text renders of the Owner, is not available in shipping build
void UCellsUtilsLibrary::ClearDisplayedCells(const UObject* Owner)
{
#if !UE_BUILD_SHIPPING
	const AActor* OwnerActor = Cast<AActor>(Owner);
	if (!OwnerActor)
	{
		const UActorComponent* Component = Cast<UActorComponent>(Owner);
		OwnerActor = Component ? Component->GetOwner() : nullptr;
		if (!ensureMsgf(OwnerActor, TEXT("ASSERT: 'OwnerActor' is null, can't Display Cells")))
		{
			return;
		}
	}

	TArray<UTextRenderComponent*> TextRendersArray;
	OwnerActor->GetComponents<UTextRenderComponent>(TextRendersArray);
	for (int32 Index = TextRendersArray.Num() - 1; Index >= 0; --Index)
	{
		UTextRenderComponent* TextRenderIt = TextRendersArray.IsValidIndex(Index) ? Cast<UTextRenderComponent>(TextRendersArray[Index]) : nullptr;
		if (IsValid(TextRenderIt)                       // is not pending kill
		    && TextRenderIt->HasAllFlags(RF_Transient)) // cell text renders have this flag
		{
			TextRenderIt->DestroyComponent();
		}
	}
#endif // !UE_BUILD_SHIPPING
}

// Display coordinates of specified cells on the level, is not available in shipping build
void UCellsUtilsLibrary::DisplayCells(UObject* Owner, const FCells& Cells, const FDisplayCellsParams& Params)
{
#if !UE_BUILD_SHIPPING
	if (!Cells.Num()
	    || !Owner)
	{
		return;
	}

	AActor* OwnerActor = Cast<AActor>(Owner);
	if (!OwnerActor)
	{
		const UActorComponent* Component = Cast<UActorComponent>(Owner);
		OwnerActor = Component ? Component->GetOwner() : nullptr;
		if (!ensureMsgf(OwnerActor, TEXT("ASSERT: 'OwnerActor' is null, can't Display Cells")))
		{
			return;
		}
	}

	if (Params.bClearPreviousDisplays)
	{
		ClearDisplayedCells(Owner);
	}

	// Have the render text rotated
	const FQuat CellGridQuaternion = Cells.Num() > 1
		                                 ? GetCellArrayRotation(Cells).Quaternion() // Get rotator from array
		                                 : GetLevelGridRotation().Quaternion();     // Get current level rotator

	for (const FCell& CellIt : Cells)
	{
		if (CellIt.IsInvalidCell())
		{
			continue;
		}

		const bool bHasAdditionalRenderString = !Params.RenderString.IsNone();
		const bool bHasCoordinatePosition = !Params.CoordinatePosition.IsZero();
		constexpr int32 MaxCoordinateRenders = 2;
		for (int32 Index = 0; Index < MaxCoordinateRenders; ++Index)
		{
			enum ERenderType { ERT_Coordinate, ERT_RenderString };
			const ERenderType RenderType = Index == 0 ? ERT_Coordinate : ERT_RenderString;
			const bool bShowRenderString = RenderType == ERT_RenderString && bHasAdditionalRenderString;
			const bool bShowCoordinate = RenderType == ERT_Coordinate && (bHasCoordinatePosition || !bHasAdditionalRenderString);

			if (!bShowCoordinate && !bShowRenderString)
			{
				continue;
			}

			UTextRenderComponent& RenderComp = *NewObject<UTextRenderComponent>(OwnerActor, NAME_None, RF_Transient);
			RenderComp.RegisterComponent();

			// Get text render location on each cell
			FVector TextLocation = FVector::ZeroVector;
			const FVector CellLocation(CellIt.X(), CellIt.Y(), GetCellHeightLocation() + Params.TextHeight);
			if (bShowCoordinate)
			{
				TextLocation.X = CellLocation.X + Params.CoordinatePosition.X * -1.f;
				TextLocation.Y = CellLocation.Y + Params.CoordinatePosition.Y * -1.f;
				TextLocation.Z = CellLocation.Z + Params.CoordinatePosition.Z;
			}
			else if (bShowRenderString)
			{
				TextLocation = CellLocation + Params.CoordinatePosition;
			}

			// --- Init the text render

			if (bShowCoordinate)
			{
				const FString XString = FString::FromInt(FMath::FloorToInt32(CellLocation.X));
				const FString YString = FString::FromInt(FMath::FloorToInt32(CellLocation.Y));
				constexpr float DelimiterTextSize = 48.f;
				const FString DelimiterString = Params.TextSize > DelimiterTextSize ? TEXT("\n") : TEXT("");
				const FString RenderString = XString + DelimiterString + YString;
				RenderComp.SetText(FText::FromString(RenderString));
			}
			else if (bShowRenderString)
			{
				// Display RenderString as is
				FText NameToStringToText = FText::FromString(Params.RenderString.ToString());
				RenderComp.SetText(NameToStringToText);
			}

			RenderComp.SetAbsolute(true, true, true);

			if (bShowCoordinate)
			{
				constexpr float CoordinateSizeMultiplier = 0.4f;
				RenderComp.SetWorldSize(Params.TextSize * CoordinateSizeMultiplier);
			}
			else if (bShowRenderString)
			{
				RenderComp.SetWorldSize(Params.TextSize);
			}

			RenderComp.SetHorizontalAlignment(EHTA_Center);
			RenderComp.SetVerticalAlignment(EVRTA_TextCenter);

			RenderComp.SetTextRenderColor(Params.TextColor.ToFColor(true));

			FTransform RenderTransform = FTransform::Identity;
			RenderTransform.SetLocation(TextLocation);
			static const FQuat AdditiveQuat = FRotator(90.f, 0.f, -90.f).Quaternion();
			RenderTransform.SetRotation(CellGridQuaternion * AdditiveQuat);
			RenderComp.SetWorldTransform(RenderTransform);
		}
	}
#endif // !UE_BUILD_SHIPPING
}

// Returns true if cells of specified actor type(s) can be displayed
bool UCellsUtilsLibrary::CanDisplayCellsForActorTypes(int32 ActorTypesBitmask)
{
#if !UE_BUILD_SHIPPING
	return (ActorTypesBitmask & AGeneratedMap::Get().DisplayCellsActorTypes) != 0;
#endif // !UE_BUILD_SHIPPING
	return false;
}
