// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Structures/Cell.h"
//---
#include "CellsUtilsLibrary.generated.h"

enum class EPathType : uint8;

/**
 * Utility structure to display cells
 * @see UCellsUtilsLibrary::DisplayCells()
 */
USTRUCT(BlueprintType)
struct BOMBER_API FDisplayCellsParams
{
	GENERATED_BODY()

	/** Default params to display cells. */
	static const FDisplayCellsParams EmptyParams;

	/** Color of displayed text.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FLinearColor TextColor = FLinearColor::White;

	/** Height offset for displayed text above the cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float TextHeight = 261.f;

	/** Size of displayed text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float TextSize = 124.f;

	/** Addition text to mark the cell, keep it short like 1-2 symbols. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FName RenderString = NAME_None;

	/** Offset for the Render String. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FVector CoordinatePosition = FVector::ZeroVector;

	/** Set true to remove all displays that were added on that owner before. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	bool bClearPreviousDisplays = false;
};

/**
 * 	The static functions library of Bomber cells (tiles on the grid).
 * 	@see trello.com/c/b2IzcOhg
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UCellsUtilsLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Exposing FCells function to blueprints
	* --------------------------------------------------- */

	/** Creates 'Make Cell' node with Cell  as an input parameter. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InVector", NativeMakeFunc, Keywords = "construct build"))
	static FORCEINLINE FCell MakeCell(double X, double Y, double Z) { return FCell(X, Y, Z); }

	/** Set the values of the cell directly. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "InVector", ScriptMethod = "SetFromVector"))
	static void Cell_SetFromVector(UPARAM(ref) FCell& InCell, const FVector& InVector) { InCell.Location = InVector; }

	/** Converts a Cell to a Vector. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InVector", DisplayName = "To Cell (Vector)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FCell Conv_VectorToCell(const FVector& InVector) { return FCell(InVector); }

	/** Converts a cell value to a string, in the form 'X= Y=' */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To String (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FString Conv_CellToString(const FCell& InCell) { return FVector2D(InCell.Location).ToString(); }

	/** Puts specified cell in the cells set.*/
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cells (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE TSet<FCell> Conv_CellToCells(const FCell& InCell) { return InCell.ToCells(); }

	/** Extracts first cell from specified cells set.*/
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cell (Cells)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FCell Conv_CellsToCell(const TSet<FCell>& InCells) { return FCell::GetFirstCellInSet(InCells); }

	/** Converts array of cells to set of cells. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cells (array)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE TSet<FCell> Conv_ArrayToCells(const TArray<FCell>& InCells) { return FCells(InCells); }

	/** Creates 'Break Cell' node with X, Y, Z outputs. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", NativeBreakFunc))
	static void BreakCell(const FCell& InCell, double& X, double& Y, double& Z);

	/** Converts a Vector to a Cell. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Vector (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static const FORCEINLINE FVector& Conv_CellToVector(const FCell& InCell) { return InCell.Location; }

	/** Returns true if cell A is equal to cell B (A == B) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "A,B", DisplayName = "Equal Exactly (Cell)", CompactNodeTitle = "==", ScriptMethod = "IsNearEqual", ScriptOperator = "==", Keywords = "== equal"))
	static FORCEINLINE bool EqualEqual_CellCell(const FCell& A, const FCell& B) { return A == B; }

	/** Returns true if cell A is not equal to cell B (A != B). */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Not Equal Exactly (Cell)", CompactNodeTitle = "!=", ScriptMethod = "IsNotNearEqual", ScriptOperator = "!=", Keywords = "!= not equal"))
	static bool NotEqual_CellCell(const FCell& A, const FCell& B) { return A != B; }

	/** Returns addition of Cell A and Cell B (A + B). */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "A,B", DisplayName = "Cell + Cell", CompactNodeTitle = "+", ScriptMethod = "Add", ScriptOperator = "+;+=", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"))
	static FORCEINLINE FCell Add_CellCell(const FCell& A, const FCell& B) { return A + B; }

	/** Returns subtraction of Cell B from Cell A (A - B). */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "A,B", DisplayName = "Cell - Cell", CompactNodeTitle = "-", ScriptMethod = "Subtract", ScriptOperator = "-;-=", Keywords = "- subtract minus"))
	static FORCEINLINE FCell Subtract_CellCell(const FCell& A, const FCell& B) { return A - B; }

	/** Returns the length of the one cell (a floor bound) */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE float GetCellSize() { return FCell::CellSize; }

	/** Cell forward direction constant (1,0,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Forward", ScriptConstantHost = "/Script/Bomber.Cell"))
	static const FORCEINLINE FCell& Cell_Forward() { return FCell::ForwardCell; }

	/** Cell backward direction constant (-1,0,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Backward", ScriptConstantHost = "/Script/Bomber.Cell"))
	static const FORCEINLINE FCell& Cell_Backward() { return FCell::BackwardCell; }

	/** Cell right direction constant (0,1,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Right", ScriptConstantHost = "/Script/Bomber.Cell"))
	static const FORCEINLINE FCell& Cell_Right() { return FCell::RightCell; }

	/** Cell left direction constant (0,-1,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Left", ScriptConstantHost = "/Script/Bomber.Cell"))
	static const FORCEINLINE FCell& Cell_Left() { return FCell::LeftCell; }

	/** Returns the invalid cell (0,0,-1) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Invalid", ScriptConstantHost = "/Script/Bomber.Cell", Keywords = "Zero"))
	static const FORCEINLINE FCell& Cell_Invalid() { return FCell::InvalidCell; }

	/** Returns true if cell is invalid (Cell == InvalidCell), to check is not the same as UCellUtillsLibrary::IsCellExistsOnLevel
	 * Some functions returns the Invalid Cell, so it could be useful to check is the cell was found or not. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", ScriptMethod = "IsInvalidCell", Keywords = "Is Zero Cell"))
	static FORCEINLINE bool Cell_IsInvalid(const FCell& Cell) { return Cell.IsInvalidCell(); }

	/** Returns true if cell is valid (Cell != InvalidCell)
	 * Some functions returns the Invalid Cell, so it could be useful to check is the cell was found or not. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", ScriptMethod = "IsValidCell", Keywords = "Is Not Zero Cell"))
	static FORCEINLINE bool Cell_IsValid(const FCell& Cell) { return Cell.IsValid(); }

	/* ---------------------------------------------------
	 *		Math library
	 * --------------------------------------------------- */

	/** Calculate the length between two cells.
	 * Could be useful to check how far two cells are far between themselves.
	 *
	 * @param C1 The first cell
	 * @param C2 The other cell
	 * @return The distance between to cells in number of cells, where each 1 unit means one cell.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "C1,C2", DisplayName = "Distance (Cell)", ScriptMethod = "Distance", Keywords = "magnitude,length"))
	static FORCEINLINE double Cell_Distance(const FCell& C1, const FCell& C2) { return FCell::Distance<double>(C1, C2); }

	/** Find the max distance between cells within specified set, where each 1 unit means one cell. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE double GetCellArrayMaxDistance(const TSet<FCell>& Cells) { return FCell::GetCellArrayMaxDistance<double>(Cells); }

	/** Finds the closest cell to the given cell within array of cells.
	 * @param Cells The array of cells to search in.
	 * @param CellToCheck The start position of the cell to check. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetCellArrayNearest(const TSet<FCell>& Cells, const FCell& CellToCheck) { return FCell::GetCellArrayNearest(Cells, CellToCheck); }

	/** Allows rotate or unrotated given grid around its origin. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE TSet<FCell> RotateCellArray(float AxisZ, const TSet<FCell>& InCells) { return FCell::RotateCellArray(AxisZ, InCells); }

#pragma region Grid
	/** Constructs and returns new grid from given transform.
	 * @param OriginTransform its location and rotation is the center of new grid, its scale-X is number of columns, scale-Y is number of rows. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE TSet<FCell> MakeCellGridByTransform(const FTransform& OriginTransform) { return FCell::MakeCellGridByTransform(OriginTransform); }

	/**Returns the cell by specified row and column number on current level if exists, invalid cell otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetCellByPositionOnGrid(const FIntPoint& CellPosition, const TSet<FCell>& InGrid) { return FCell::GetCellByPositionOnGrid(CellPosition, InGrid); }

	/** Takes the cell and returns its column (X) and row (Y) position on given grid if exists, -1 otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FIntPoint GetPositionByCellOnGrid(const FCell& InCell, const TSet<FCell>& InGrid) { return FCell::GetPositionByCellOnGrid(InCell, InGrid); }

	/** Returns the center column (X) and row (Y) position on given grid.
	  * E.g: for grid with 5 rows and 5 columns, the center cell will be (2,2). */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FIntPoint GetCenterCellPositionOnGrid(const TSet<FCell>& InGrid) { return FCell::GetCenterCellPositionOnGrid(InGrid); }

	/** Returns 4 corner cells on given cells grid. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE TSet<FCell> GetCornerCellsOnGrid(const TSet<FCell>& InGrid) { return FCell::GetCornerCellsOnGrid(InGrid); }

	/** Returns specified corner cell in given grid. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetCellByCornerOnGrid(EGridCorner CornerType, const TSet<FCell>& InGrid) { return FCell::GetCellByCornerOnGrid(CornerType, InGrid); }

	/** Scales specified cell maintaining relative distance from the corners of the new grid.
	 * @param OriginalCell The cell to scale.
	 * @param NewCornerCells The new corner cells to scale to.
	 * @return scaled cell, is not aligned to any existed cell, make sure to snap it to the grid. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell ScaleCellToNewGrid(const FCell& OriginalCell, const TSet<FCell>& NewCornerCells) { return FCell::ScaleCellToNewGrid(OriginalCell, NewCornerCells); }
#pragma endregion Grid

#pragma region Transform
	/** Makes origin transform for given grid. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FTransform GetCellArrayTransform(const TSet<FCell>& InCells) { return FCell::GetCellArrayTransform(InCells); }

	/** Find the average of an set of cells. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "location"))
	static FORCEINLINE FCell GetCellArrayCenter(const TSet<FCell>& Cells) { return FCell::GetCellArrayCenter(Cells); }

	/** Makes rotator for given grid its origin. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FRotator GetCellArrayRotation(const TSet<FCell>& InCells) { return FCell::GetCellArrayRotation(InCells); }

	/** Returns the width (columns X) and the length (rows Y) in specified cells, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 7 columns (X) and 9 rows (Y) columns respectively. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "scale"))
	static FORCEINLINE FVector2D GetCellArraySize(const TSet<FCell>& InCells) { return FCell::GetCellArraySize(InCells); }

	/** Returns number of columns (X) in specified cells array, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 7 width that represent columns (X). */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "size,scale"))
	static FORCEINLINE float GetCellArrayWidth(const TSet<FCell>& InCells) { return FCell::GetCellArrayWidth(InCells); }

	/** Returns number of rows (Y) in specified cells array, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 9 length that represent rows (Y). */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "size,scale"))
	static FORCEINLINE float GetCellArrayLength(const TSet<FCell>& InCells) { return FCell::GetCellArrayLength(InCells); }
#pragma endregion Transform

	/* ---------------------------------------------------
	*		Grid transform library
	* --------------------------------------------------- */

	/** Returns transform of cells grid on current level, where:
	 * Transform location and rotation is the center of the grid.
	 * Transform scale-X is number of columns (width).
	 * Transform scale-Y is number of rows (length). */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FTransform GetLevelGridTransform();

#pragma region Location
	/** Returns location of grid pivot on current level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FVector GetLevelGridLocation();

	/** Returns any cell Z-location on the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Z,grid"))
	static float GetCellHeightLocation();
#pragma endregion Location

#pragma region Rotation
	/** Returns cell rotator that is the same for any cell on the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FRotator GetLevelGridRotation();

	/** Returns cell yaw angle in degrees that is the same for any cell on the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "rotation,grid"))
	static float GetCellYawDegree();
#pragma endregion Rotation

#pragma region Scale
	/** Returns the current grid size, where:
	 * scale-X is number of columns (width),
	 * scale-Y is number of rows (length). */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "size"))
	static FIntPoint GetLevelGridScale();

	/** Returns the width (number of columns X) of the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "width,X,size,scale,grid"))
	static int32 GetCellColumnsNumOnLevel();

	/** Returns the length (number of rows Y) of the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "length,Y,size,scale,grid"))
	static int32 GetCellRowsNumOnLevel();

	/** Returns GetCellColumnsNumOnLevel - 1. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "width,X,size,scale,grid"))
	static int32 GetLastColumnIndexOnLevel();

	/** Returns Returns GetCellRowsNumOnLevel - 1. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "length,Y,size,scale,grid"))
	static int32 GetLastRowIndexOnLevel();
#pragma endregion Scale

	/* ---------------------------------------------------
	 *		Generated Map related cell functions
	 * --------------------------------------------------- */

	/** Returns the cell by specified column (X) and row (Y) on current level if exists, invalid cell otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetCellByPositionOnLevel(int32 ColumnX, int32 RowY) { return GetCellByPositionOnGrid(FIntPoint(ColumnX, RowY), GetAllCellsOnLevel()); }

	/** Takes the cell and returns its column (X) and row (Y) position on current level if exists, -1 otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell"))
	static void GetPositionByCellOnLevel(const FCell& InCell, int32& OutColumnX, int32& OutRowY);

	/** Returns all grid cell locations on the Generated Map as Set. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE TSet<FCell> GetAllCellsOnLevel() { return FCells{GetAllCellsOnLevelAsArray()}; }

	/** Returns all grid cell locations on the Generated Map as Array. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const TArray<FCell>& GetAllCellsOnLevelAsArray();

	/** Returns the cell location of the Generated Map.
	 * Could be useful to get always center cell regardless of the location of the Generated Map in the world. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FCell GetCenterCellOnLevel();

	/** Returns the center row and column positions on the level.
	 * E.g: for level with 5 rows and 5 columns, the center cell will be (2,2). */
	UFUNCTION(BlueprintPure, Category = "C++")
	static void GetCenterCellPositionOnLevel(int32& OutColumnX, int32& OutRowY);

#pragma region CornerCell
	/** Returns 4 corner cells of the Generated Map respecting its current size. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE TSet<FCell> GetCornerCellsOnLevel() { return GetCornerCellsOnGrid(GetAllCellsOnLevel()); }

	/** Returns specified corner cell in given grid. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetCellByCornerOnLevel(EGridCorner CornerType) { return GetCellByCornerOnGrid(CornerType, GetAllCellsOnLevel()); }

	/** Returns true if given cell is corner cell of current level. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FORCEINLINE bool IsCornerCellOnLevel(const FCell& Cell) { return GetCornerCellsOnLevel().Contains(Cell); }

	/** Return closest corner cell to the given cell.
	 * @param CellToCheck The start position of the cell to check. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CellToCheck"))
	static FCell GetNearestCornerCellOnLevel(const FCell& CellToCheck);
#pragma endregion CornerCell

	/** Returns all empty grid cell locations on the Generated Map where non of actors are present. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Free"))
	static TSet<FCell> GetAllEmptyCellsWithoutActors();

	/** Returns all grid cell locations on the Generated Map by specified actor types.
	 * If non of actors are chosen, returns all empty cells without actors. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Cell By Actor"))
	static TSet<FCell> GetAllCellsWithActors(
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Takes cells and returns only empty cells where non of actors are present.
	 * Could be useful to extract only free no actor cells with within given cells. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Free"))
	static TSet<FCell> FilterEmptyCellsWithoutActors(const TSet<FCell>& InCells);

	/** Takes cells and returns only matching with specified actor types.
	 * If non of actors are chosen, returns matching empty cells without actors.
	 * Could be useful to extract only items within given cells.
	 *
	 * @param InCells Cells to filter.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 * @return cells with actors of specified types.
	 */
	UFUNCTION(BlueprintPure, Category = "C++")
	static TSet<FCell> FilterCellsByActors(
		const TSet<FCell>& InCells,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if specified cell is empty, so it does not have own actor.
	* Could be useful to make sure there is nothing on cell, so some actor could be spawned there. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "Free"))
	static bool IsEmptyCellWithoutActor(const FCell& Cell);

	/** Returns true if a cell has an actor of specified type (or its type matches with at least one type if put more than one type).
	 * If non of actors are chosen, then returns true if specified cell is empty, so it does not have own actor.
	 * Could be useful to determine does input cell contain specific actor in itself like wall, so there is no way. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static bool IsCellHasAnyMatchingActor(
		const FCell& Cell,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if at least one cell along specified is empty, so it does not have own actor.*/
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "Free"))
	static bool IsAnyCellEmptyWithoutActor(const TSet<FCell>& Cells);

	/** Returns true if at least one cell has actors of specified types.
	 * If non of actors are chosen, then returns true if at least one cell along specified is empty, so it does not have own actor.
	 * Could be useful to determine do input cells contain at least one item. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool AreCellsHaveAnyMatchingActors(
		const TSet<FCell>& Cells,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if all specified cells are empty, so don't have own actors.
	 * Could be useful to make sure there are nothing on cells, so some actors could be spawned there. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "Free"))
	static bool AreAllCellsEmptyWithoutActors(const TSet<FCell>& Cells);

	/** Returns true if all cells have actors of specified types.
	 * If non of actors are chosen, then returns true if all specified cells are empty, so don't have own actors.
	 * Could be useful to make sure there only players on input cells. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool AreCellsHaveAllMatchingActors(
		const TSet<FCell>& Cells,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if specified cell is present on the Generated Map.
	 * Could be useful to check is input cell valid. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "Valid"))
	static FORCEINLINE bool IsCellExistsOnLevel(const FCell& Cell) { return Cell.IsValid() && GetAllCellsOnLevel().Contains(Cell); }

	/** Returns true if the cell is present on the Generated Map with such row and column indexes.
	 * Could be useful to check row and and column. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Valid"))
	static FORCEINLINE bool IsCellPositionExistsOnLevel(int32 ColumnX, int32 RowY) { return GetCellByPositionOnLevel(ColumnX, RowY).IsValid(); }

	/** Returns true if at least one cell is present on the Generated Map.*/
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Valid"))
	static FORCEINLINE bool IsAnyCellExistsOnLevel(const TSet<FCell>& Cells) { return GetAllCellsOnLevel().Intersect(Cells).Num() > 0; }

	/** Returns true if all specified cells are present on the Generated Map.
	 * Could be useful to determine are all input cells valid. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "valid"))
	static FORCEINLINE bool AreAllCellsExistOnLevel(const TSet<FCell>& Cells) { return GetAllCellsOnLevel().Includes(Cells); }

	/** Returns cells around the center in specified radius and according desired type of breaks.
	 * Could be useful to find all possible ways around.
	 *
	 * @param CenterCell The start of searching in all directions.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Side"))
	static TSet<FCell> GetCellsAround(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 Radius);

	/** Returns cells that match specified actors in specified radius from a center, according desired type of breaks.
	 * If non of actors are chosen, returns matching empty cells around without actors.
	 * Could be useful to determine are there any players or items around.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Side"))
	static TSet<FCell> GetCellsAroundWithActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 Radius,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns matching empty cells around without actors, according desired type of breaks.
	 * Could be useful to determine are there empty cells around.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Free,Side"))
	static TSet<FCell> GetEmptyCellsAroundWithoutActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 Radius);

	/** Returns first cell in specified direction from a center, according desired type of breaks.
	 * Could be useful to get next cell.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Pathfinder Type of cells searching.
	 * @param Direction Side to find the cells.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell,Side"))
	static FCell GetCellInDirection(
		const FCell& CenterCell,
		EPathType Pathfinder,
		ECellDirection Direction);

	/** Returns true if a cell was found in specified direction from a center, according desired type of breaks.
	 * Could be useful to determine can the next cell be taken.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Pathfinder Type of cells searching.
	 * @param Direction Side to find the cells.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell,Side"))
	static bool CanGetCellInDirection(
		const FCell& CenterCell,
		EPathType Pathfinder,
		ECellDirection Direction);

	/** Returns cells in specified direction from a center, according desired type of breaks.
	 * Could be useful to get horizontal cells in left and right direction or vertical cells in forward and backward directions.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell,Side"))
	static TSet<FCell> GetCellsInDirections(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.ECellDirection")) int32 DirectionsBitmask);

	/** Returns cells that match specified actors in specified direction from a center, according desired type of breaks.
	 * If non of actors are chosen, returns matching empty cells without actors in chosen direction(s).
	 * Could be useful to determine are there any players or items on the way.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Side"))
	static TSet<FCell> GetCellsInDirectionsWithActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.ECellDirection")) int32 DirectionsBitmask,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns matching empty cells without actors in chosen direction(s), according desired type of breaks.
	 * Could be useful to determine are there empty cells on the way.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Free,Side"))
	static TSet<FCell> GetEmptyCellsInDirectionsWithoutActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.ECellDirection")) int32 DirectionsBitmask);

	/** Returns true if player is not able to reach specified cell by any any path. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "Path"))
	static bool IsIslandCell(const FCell& Cell);

	/** Gets a copy of given cell rotated around the given transform to the same yaw degree.
	 * @param Cell - The cell to rotate.
	 * @param AxisZ The Z param of the axis to rotate around. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FCell RotateCellAroundLevelOrigin(const FCell& Cell, float AxisZ);

	/** Gets a copy of given cell snapped to nearest cell on the level grid. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", Keywords = "Grid Snap,near"))
	static FORCEINLINE FCell SnapCellOnLevel(const FCell& Cell) { return GetCellArrayNearest(GetAllCellsOnLevel(), Cell); }

	/** Returns nearest free cell to given cell, where free means cell with no other level actors except players. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FCell GetNearestFreeCell(const FCell& Cell);

	/* ---------------------------------------------------
	 *		Debug cells utilities
	 * --------------------------------------------------- */

	/** Remove all text renders of the Owner, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static void ClearDisplayedCells(const UObject* Owner);

	/** Display coordinates of specified cells on the level, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "Params"))
	static void DisplayCells(UObject* Owner, const TSet<FCell>& Cells, const FDisplayCellsParams& Params);

	/** Display only specified cell, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "Params"))
	static void DisplayCell(UObject* Owner, const FCell& Cell, const FDisplayCellsParams& Params) { DisplayCells(Owner, {Cell}, Params); }

	/** Returns true if cells of specified actor type(s) can be displayed.
	 * It takes into considerations the types that are set by 'Bomber.Debug.DisplayCells' cheat or directly in the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool CanDisplayCellsForActorTypes(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorTypesBitmask);
};
