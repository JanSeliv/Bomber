// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

// Bomber
#include "Structures/BmrCell.h"

#include "BmrCellUtilsLibrary.generated.h"

enum class EPathType : uint8;

/**
 * Utility structure to display cells
 * @see UBmrCellUtilsLibrary::DisplayCells()
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrDisplayCellsParams
{
	GENERATED_BODY()

	/** Default params to display cells. */
	static const FBmrDisplayCellsParams EmptyParams;

	/** Color of displayed text.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	FLinearColor TextColor = FLinearColor::White;

	/** Height offset for displayed text above the cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	float TextHeight = 273.f;

	/** Size of displayed text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	float TextSize = 124.f;

	/** Addition text to mark the cell, keep it short like 1-2 symbols. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	FName RenderString = NAME_None;

	/** Offset for the Render String. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	FVector CoordinatePosition = FVector::ZeroVector;

	/** Set true to remove all displays that were added on that owner before. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	bool bClearPreviousDisplays = false;
};

/**
 * 	The static functions library of Bomber cells (tiles on the grid).
 * 	@see trello.com/c/b2IzcOhg
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrCellUtilsLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Conversions
	 ********************************************************************************************* */
public:
	/** Creates 'Make Cell' node with Cell  as an input parameter. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InVector", NativeMakeFunc, Keywords = "construct build"))
	static FORCEINLINE FBmrCell MakeCell(double X, double Y, double Z) { return FBmrCell(X, Y, Z); }

	/** Set the values of the cell directly. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InVector", ScriptMethod = "SetFromVector"))
	static void Cell_SetFromVector(UPARAM(ref) FBmrCell& InCell, const FVector& InVector) { InCell.Location = InVector; }

	/** Converts a Cell to a Vector. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InVector", DisplayName = "To Cell (Vector)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FBmrCell Conv_VectorToCell(const FVector& InVector) { return FBmrCell(InVector); }

	/** Converts a cell value to a string, in the form `X= Y=` */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To String (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FString Conv_CellToString(const FBmrCell& InCell) { return InCell.ToString(); }

	/** Puts specified cell in the cells set.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cells (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE TSet<FBmrCell> Conv_CellToCells(const FBmrCell& InCell) { return InCell.ToCells(); }

	/** Extracts first cell from specified cells set.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cell (Cells)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FBmrCell Conv_CellsToCell(const TSet<FBmrCell>& InCells) { return FBmrCell::GetFirstCellInSet(InCells); }

	/** Converts array of cells to set of cells. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cells (array)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE TSet<FBmrCell> Conv_ArrayToCells(const TArray<FBmrCell>& InCells) { return FBmrCells(InCells); }

	/** Creates 'Break Cell' node with X, Y, Z outputs. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", NativeBreakFunc))
	static void BreakCell(const FBmrCell& InCell, double& X, double& Y, double& Z);

	/** Converts a Vector to a Cell. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Vector (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static const FORCEINLINE FVector& Conv_CellToVector(const FBmrCell& InCell) { return InCell.Location; }

	/** Converts a set of cells to an array of vectors. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCells", DisplayName = "To Vectors (Cells)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE TArray<FVector> Conv_CellsToVectors(const TSet<FBmrCell>& InCells) { return FBmrCell::CellsToVectors(InCells); }

	/** Converts an array of vectors to a set of cells. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InVectors", DisplayName = "To Cells (Vectors)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE TSet<FBmrCell> Conv_VectorsToCells(const TArray<FVector>& InVectors) { return FBmrCell::VectorsToCells(InVectors); }

	/** Returns true if cell A is equal to cell B (A == B) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "A,B", DisplayName = "Equal Exactly (Cell)", CompactNodeTitle = "==", ScriptMethod = "IsNearEqual", ScriptOperator = "==", Keywords = "== equal"))
	static FORCEINLINE bool EqualEqual_CellCell(const FBmrCell& A, const FBmrCell& B) { return A == B; }

	/** Returns true if cell A is not equal to cell B (A != B). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "Not Equal Exactly (Cell)", CompactNodeTitle = "!=", ScriptMethod = "IsNotNearEqual", ScriptOperator = "!=", Keywords = "!= not equal"))
	static bool NotEqual_CellCell(const FBmrCell& A, const FBmrCell& B) { return A != B; }

	/** Returns addition of Cell A and Cell B (A + B). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "A,B", DisplayName = "Cell + Cell", CompactNodeTitle = "+", ScriptMethod = "Add", ScriptOperator = "+;+=", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"))
	static FORCEINLINE FBmrCell Add_CellCell(const FBmrCell& A, const FBmrCell& B) { return A + B; }

	/** Returns subtraction of Cell B from Cell A (A - B). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "A,B", DisplayName = "Cell - Cell", CompactNodeTitle = "-", ScriptMethod = "Subtract", ScriptOperator = "-;-=", Keywords = "- subtract minus"))
	static FORCEINLINE FBmrCell Subtract_CellCell(const FBmrCell& A, const FBmrCell& B) { return A - B; }

	/** Returns the length of the one cell (a floor bound) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE float GetCellSize() { return FBmrCell::CellSize; }

	/** Cell forward direction constant (1,0,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (ScriptConstant = "Forward", ScriptConstantHost = "/Script/Bomber.BmrCell"))
	static const FORCEINLINE FBmrCell& Cell_Forward() { return FBmrCell::ForwardCell; }

	/** Cell backward direction constant (-1,0,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (ScriptConstant = "Backward", ScriptConstantHost = "/Script/Bomber.BmrCell"))
	static const FORCEINLINE FBmrCell& Cell_Backward() { return FBmrCell::BackwardCell; }

	/** Cell right direction constant (0,1,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (ScriptConstant = "Right", ScriptConstantHost = "/Script/Bomber.BmrCell"))
	static const FORCEINLINE FBmrCell& Cell_Right() { return FBmrCell::RightCell; }

	/** Cell left direction constant (0,-1,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (ScriptConstant = "Left", ScriptConstantHost = "/Script/Bomber.BmrCell"))
	static const FORCEINLINE FBmrCell& Cell_Left() { return FBmrCell::LeftCell; }

	/** Returns the invalid cell (0,0,-1) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (ScriptConstant = "Invalid", ScriptConstantHost = "/Script/Bomber.BmrCell", Keywords = "Zero"))
	static const FORCEINLINE FBmrCell& Cell_Invalid() { return FBmrCell::InvalidCell; }

	/** Returns true if cell is invalid (Cell == InvalidCell), to check is not the same as UBmrCellUtilsLibrary::IsCellExistsOnLevel
	 * Some functions returns the Invalid Cell, so it could be useful to check is the cell was found or not. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", ScriptMethod = "IsInvalidCell", Keywords = "Is Zero Cell"))
	static FORCEINLINE bool Cell_IsInvalid(const FBmrCell& Cell) { return Cell.IsInvalidCell(); }

	/** Returns true if cell is valid (Cell != InvalidCell)
	 * Some functions returns the Invalid Cell, so it could be useful to check is the cell was found or not. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", ScriptMethod = "IsValidCell", Keywords = "Is Not Zero Cell"))
	static FORCEINLINE bool Cell_IsValid(const FBmrCell& Cell) { return Cell.IsValid(); }

	/*********************************************************************************************
	 * Grid (Array of cells)
	 * Exposed FBmrCell math functions to blueprints
	 ********************************************************************************************* */
public:
	/** Calculate the length between two cells.
	 * Could be useful to check how far two cells are far between themselves.
	 *
	 * @param C1 The first cell
	 * @param C2 The other cell
	 * @return The distance between to cells in number of cells, where each 1 unit means one cell.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "C1,C2", DisplayName = "Distance (Cell)", ScriptMethod = "Distance", Keywords = "magnitude,length"))
	static FORCEINLINE double Cell_Distance(const FBmrCell& C1, const FBmrCell& C2) { return FBmrCell::Distance<double>(C1, C2); }

	/** Find the max distance between cells within specified set, where each 1 unit means one cell. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE double GetCellArrayMaxDistance(const TSet<FBmrCell>& Cells) { return FBmrCell::GetCellArrayMaxDistance<double>(Cells); }

	/** Finds the closest cell to the given cell within array of cells.
	 * @param Cells The array of cells to search in.
	 * @param CellToCheck The start position of the cell to check. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FBmrCell GetCellArrayNearest(const TSet<FBmrCell>& Cells, const FBmrCell& CellToCheck) { return FBmrCell::GetCellArrayNearest(Cells, CellToCheck); }

	/** Allows to rotate or unrotated given grid around its origin. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE TSet<FBmrCell> RotateCellArray(float AxisZ, const TSet<FBmrCell>& InCells) { return FBmrCell::RotateCellArray(AxisZ, InCells); }

	/** Constructs and returns new grid from given transform.
	 * @param OriginTransform its location and rotation is the center of new grid, its scale-X is number of columns, scale-Y is number of rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE TSet<FBmrCell> MakeCellGridByTransform(const FTransform& OriginTransform) { return FBmrCell::MakeCellGridByTransform(OriginTransform); }

	/**Returns the cell by specified row and column number on current level if exists, invalid cell otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FBmrCell GetCellByPositionOnGrid(const FIntPoint& CellPosition, const TSet<FBmrCell>& InGrid) { return FBmrCell::GetCellByPositionOnGrid(CellPosition, InGrid); }

	/** Takes the cell and returns its column (X) and row (Y) position on given grid if exists, -1 otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FIntPoint GetPositionByCellOnGrid(const FBmrCell& InCell, const TSet<FBmrCell>& InGrid) { return FBmrCell::GetPositionByCellOnGrid(InCell, InGrid); }

	/** Returns the center column (X) and row (Y) position on given grid.
	 * E.g: for grid with 5 rows and 5 columns, the center cell will be (2,2). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FIntPoint GetCenterCellPositionOnGrid(const TSet<FBmrCell>& InGrid) { return FBmrCell::GetCenterCellPositionOnGrid(InGrid); }

	/** Returns 4 corner cells on given cells grid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE TSet<FBmrCell> GetCornerCellsOnGrid(const TSet<FBmrCell>& InGrid) { return FBmrCell::GetCornerCellsOnGrid(InGrid); }

	/** Returns specified corner cell in given grid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FBmrCell GetCellByCornerOnGrid(EBmrGridCorner CornerType, const TSet<FBmrCell>& InGrid) { return FBmrCell::GetCellByCornerOnGrid(CornerType, InGrid); }

	/** Scales specified cell maintaining relative distance from the corners of the new grid.
	 * @param OriginalCell The cell to scale.
	 * @param NewCornerCells The new corner cells to scale to.
	 * @return scaled cell, is not aligned to any existed cell, make sure to snap it to the grid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FBmrCell ScaleCellToNewGrid(const FBmrCell& OriginalCell, const TSet<FBmrCell>& NewCornerCells) { return FBmrCell::ScaleCellToNewGrid(OriginalCell, NewCornerCells); }

	/** Makes origin transform for given grid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FTransform GetCellArrayTransform(const TSet<FBmrCell>& InCells) { return FBmrCell::GetCellArrayTransform(InCells); }

	/** Find the average of a set of cells. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "location"))
	static FORCEINLINE FBmrCell GetCellArrayCenter(const TSet<FBmrCell>& Cells) { return FBmrCell::GetCellArrayCenter(Cells); }

	/** Makes rotator for given grid its origin. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FRotator GetCellArrayRotation(const TSet<FBmrCell>& InCells) { return FBmrCell::GetCellArrayRotation(InCells); }

	/** Returns the width (columns X) and the length (rows Y) in specified cells, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 7 columns (X) and 9 rows (Y) columns respectively. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "scale"))
	static FORCEINLINE FVector2D GetCellArraySize(const TSet<FBmrCell>& InCells) { return FBmrCell::GetCellArraySize(InCells); }

	/** Returns number of columns (X) in specified cells array, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 7 width that represent columns (X). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "size,scale"))
	static FORCEINLINE float GetCellArrayWidth(const TSet<FBmrCell>& InCells) { return FBmrCell::GetCellArrayWidth(InCells); }

	/** Returns number of rows (Y) in specified cells array, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 9 length that represent rows (Y). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "size,scale"))
	static FORCEINLINE float GetCellArrayLength(const TSet<FBmrCell>& InCells) { return FBmrCell::GetCellArrayLength(InCells); }

	/** Keeps cells within range of the StartingCell and avoids barriers.
	 * E.g: might be useful to exclude all explosions cells and those that are out of explosions, so the bot (Starting Cell) will not attempt to go through explosions.
	 * @param ActiveCells The cells to process and filter.
	 * @param BoundaryCells The cells acting as barriers.
	 * @param StartingCell The reference cell for proximity and direction.
	 * @return A set of filtered cells (`FBmrCells`) that meet the criteria. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "StartingCell"))
	static FORCEINLINE TSet<FBmrCell> FilterCellsByBounds(const TSet<FBmrCell>& ActiveCells, const TSet<FBmrCell>& BoundaryCells, const FBmrCell& StartingCell) { return FBmrCell::FilterCellsByBounds(ActiveCells, BoundaryCells, StartingCell); }

	/** Returns true if the Starting Cell is in line of sight (in the direction of the Target Cell) within the given angle when comparing cells on the grid.
	 * Might be useful for AI to check does it see the bomb or player in the line of sight.
	 * @param StartingCell The reference starting cell (who is looking).
	 * @param TargetCell The target cell to check direction to (that can be seen).
	 * @param AllVisibleCells All cells that can be seen from the starting cell, the grid or just part of it.
	 * @param MaxAngleDegrees The maximum allowable angle (in degrees) for alignment, is recommended around 20 degrees. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "StartingCell,TargetCell"))
	static FORCEINLINE bool CanCellSeeTarget(const FBmrCell& StartingCell, const FBmrCell& TargetCell, const TSet<FBmrCell>& AllVisibleCells, float MaxAngleDegrees) { return FBmrCell::CanCellSeeTarget(StartingCell, TargetCell, AllVisibleCells, MaxAngleDegrees); }

	/*********************************************************************************************
	 * Transform (Location, Rotation, Scale) on the level
	 ********************************************************************************************* */
public:
	/** Returns transform of cells grid on current level, where:
	 * Transform location and rotation is the center of the grid.
	 * Transform scale-X is number of columns (width).
	 * Transform scale-Y is number of rows (length). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FTransform GetLevelGridTransform();

#pragma region Location
	/** Returns location of grid pivot on current level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FVector GetLevelGridLocation();

	/** Returns any cell Z-location on the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Z,grid"))
	static float GetCellHeightLocation();
#pragma endregion Location

#pragma region Rotation
	/** Returns cell rotator that is the same for any cell on the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FRotator GetLevelGridRotation();

	/** Returns cell yaw angle in degrees that is the same for any cell on the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "rotation,grid"))
	static float GetCellYawDegree();
#pragma endregion Rotation

#pragma region Scale
	/** Returns the current grid size, where:
	 * scale-X is number of columns (width),
	 * scale-Y is number of rows (length). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "size"))
	static FIntPoint GetLevelGridScale();

	/** Returns the width (number of columns X) of the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "width,X,size,scale,grid"))
	static int32 GetCellColumnsNumOnLevel();

	/** Returns the length (number of rows Y) of the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "length,Y,size,scale,grid"))
	static int32 GetCellRowsNumOnLevel();

	/** Returns GetCellColumnsNumOnLevel - 1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "width,X,size,scale,grid"))
	static int32 GetLastColumnIndexOnLevel();

	/** Returns GetCellRowsNumOnLevel - 1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "length,Y,size,scale,grid"))
	static int32 GetLastRowIndexOnLevel();
#pragma endregion Scale

	/*********************************************************************************************
	 * Generated Map related cell functions
	 ********************************************************************************************* */
public:
	/** Returns the cell by specified column (X) and row (Y) on current level if exists, invalid cell otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FBmrCell GetCellByPositionOnLevel(int32 ColumnX, int32 RowY) { return GetCellByPositionOnGrid(FIntPoint(ColumnX, RowY), GetAllCellsOnLevel()); }

	/** Takes the cell and returns its column (X) and row (Y) position on current level if exists, -1 otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell"))
	static void GetPositionByCellOnLevel(const FBmrCell& InCell, int32& OutColumnX, int32& OutRowY);

	/** Returns array index for cell in grid data structure, otherwise -1 if not found.
	 * Might be useful as cell ID for networking or data association (e.g. cell(200,400) at position [1,2] has index 15) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell"))
	static int32 GetIndexByCellOnLevel(const FBmrCell& InCell);

	/** Gets cell from array index in grid data structure, otherwise invalid cell if not found.
	 * Might be useful as cell ID for networking or data association (e.g. index 15 → cell(200,400) at position [1,2]) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FBmrCell GetCellByIndexOnLevel(int32 CellIndex);

	/** Returns all grid cell locations on the Generated Map as Set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE TSet<FBmrCell> GetAllCellsOnLevel() { return FBmrCells{GetAllCellsOnLevelAsArray()}; }

	/** Returns all grid cell locations on the Generated Map as Array. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const TArray<FBmrCell>& GetAllCellsOnLevelAsArray();

	/** Returns the cell location of the Generated Map.
	 * Could be useful to get always center cell regardless of the location of the Generated Map in the world. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FBmrCell GetCenterCellOnLevel();

	/** Returns the center row and column positions on the level.
	 * E.g: for level with 5 rows and 5 columns, the center cell will be (2,2). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static void GetCenterCellPositionOnLevel(int32& OutColumnX, int32& OutRowY);
	static FIntPoint GetCenterCellPositionOnLevel();

	/** Returns all empty grid cell locations on the Generated Map where none of actors are present. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Free"))
	static TSet<FBmrCell> GetAllEmptyCellsWithoutActors();

	/** Returns all grid cell locations on the Generated Map by specified actor types.
	 * If none of actors are chosen, returns all empty cells without actors. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Cell By Actor"))
	static TSet<FBmrCell> GetAllCellsWithActors(
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** The intersection of (OutCells ∩ ActorsTypesBitmask).
	 * @warning Is not public blueprintable since all related ufunctions are already use this method, they all do the same e.g: FilterCellsByActors, IsCellHasAnyMatchingActor etc.
	 * @param InOutCells Will contain cells with actors of specified types.
	 * @param ActorsTypesBitmask Bitmask of actors types to intersect.
	 * @param bIntersectAllIfEmpty If the specified set is empty, then all non-empty cells of each actor will be iterated as a source set.
	 */
	static void IntersectCellsByTypes(FBmrCells& InOutCells, int32 ActorsTypesBitmask, bool bIntersectAllIfEmpty);

	/** Takes cells and returns only empty cells where none of actors are present.
	 * Could be useful to extract only free no actor cells with within given cells. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Free"))
	static TSet<FBmrCell> FilterEmptyCellsWithoutActors(const TSet<FBmrCell>& InCells);

	/** Takes cells and returns only matching with specified actor types.
	 * If none of actors are chosen, returns matching empty cells without actors.
	 * Could be useful to extract only items within given cells.
	 *
	 * @param InCells Cells to filter.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 * @return cells with actors of specified types.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static TSet<FBmrCell> FilterCellsByActors(
	    const TSet<FBmrCell>& InCells,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns true if specified cell is empty, so it does not have own actor.
	 * Could be useful to make sure there is nothing on cell, so some actor could be spawned there. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", Keywords = "Free"))
	static bool IsEmptyCellWithoutActor(const FBmrCell& Cell);

	/** Returns true if a cell has an actor of specified type (or its type matches with at least one type if put more than one type).
	 * If none of actors are chosen, then returns true if specified cell is empty, so it does not have own actor.
	 * Could be useful to determine does input cell contain specific actor in itself like wall, so there is no way. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell"))
	static bool IsCellHasAnyMatchingActor(
	    const FBmrCell& Cell,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns true if at least one cell along specified is empty, so it does not have own actor.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", Keywords = "Free"))
	static bool IsAnyCellEmptyWithoutActor(const TSet<FBmrCell>& Cells);

	/** Returns true if at least one cell has actors of specified types.
	 * If none of actors are chosen, then returns true if at least one cell along specified is empty, so it does not have own actor.
	 * Could be useful to determine do input cells contain at least one item. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static bool AreCellsHaveAnyMatchingActors(
	    const TSet<FBmrCell>& Cells,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns true if all specified cells are empty, so don't have own actors.
	 * Could be useful to make sure there are nothing on cells, so some actors could be spawned there. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", Keywords = "Free"))
	static bool AreAllCellsEmptyWithoutActors(const TSet<FBmrCell>& Cells);

	/** Returns true if all cells have actors of specified types.
	 * If none of actors are chosen, then returns true if all specified cells are empty, so don't have own actors.
	 * Could be useful to make sure there only players on input cells. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static bool AreCellsHaveAllMatchingActors(
	    const TSet<FBmrCell>& Cells,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns true if specified cell is present on the Generated Map.
	 * Could be useful to check is input cell valid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", Keywords = "Valid"))
	static FORCEINLINE bool IsCellExistsOnLevel(const FBmrCell& Cell) { return Cell.IsValid() && GetAllCellsOnLevel().Contains(Cell); }

	/** Returns true if the cell is present on the Generated Map with such row and column indexes.
	 * Could be useful to check row and column. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Valid"))
	static FORCEINLINE bool IsCellPositionExistsOnLevel(int32 ColumnX, int32 RowY) { return GetCellByPositionOnLevel(ColumnX, RowY).IsValid(); }

	/** Returns true if at least one cell is present on the Generated Map.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Valid"))
	static FORCEINLINE bool IsAnyCellExistsOnLevel(const TSet<FBmrCell>& Cells) { return GetAllCellsOnLevel().Intersect(Cells).Num() > 0; }

	/** Returns true if all specified cells are present on the Generated Map.
	 * Could be useful to determine are all input cells valid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "valid"))
	static FORCEINLINE bool AreAllCellsExistOnLevel(const TSet<FBmrCell>& Cells) { return GetAllCellsOnLevel().Includes(Cells); }

	/** Getting an array of cells by any sides from an input center cell and type of breaks.
	 * @warning Is not public blueprintable since all related ufunctions are already use this method, they all do the same e.g: GetCellsAround, GetCellInDirection etc.
	 *
	 * @param OutCells Will contain found cells.
	 * @param Cell The start of searching by the sides.
	 * @param Pathfinder Type of cells searching.
	 * @param SideLength Distance in number of cells from a center.
	 * @param DirectionsBitmask All sides need to iterate.
	 * @param bBreakInputCells In case, specified OutCells is not empty, these cells break lines as the Wall behavior, will not be removed from the array.
	 */
	static void GetSideCells(
	    FBmrCells& OutCells,
	    const FBmrCell& Cell,
	    EPathType Pathfinder,
	    int32 SideLength,
	    int32 DirectionsBitmask,
	    bool bBreakInputCells = false);

	/** Returns cells around the center in specified radius and according desired type of breaks.
	 * Could be useful to find all possible ways around.
	 *
	 * @param CenterCell The start of searching in all directions.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Side"))
	static TSet<FBmrCell> GetCellsAround(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    int32 Radius);

	/** Returns cells that match specified actors in specified radius from a center, according desired type of breaks.
	 * If none of actors are chosen, returns matching empty cells around without actors.
	 * Could be useful to determine are there any players or items around.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Side"))
	static TSet<FBmrCell> GetCellsAroundWithActors(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    int32 Radius,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns matching empty cells around without actors, according desired type of breaks.
	 * Could be useful to determine are there empty cells around.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Free,Side"))
	static TSet<FBmrCell> GetEmptyCellsAroundWithoutActors(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    int32 Radius);

	/** Returns first cell in specified direction from a center, according desired type of breaks.
	 * Could be useful to get next cell.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Pathfinder Type of cells searching.
	 * @param Direction Side to find the cells.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell,Side"))
	static FBmrCell GetCellInDirection(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    EBmrCellDirection Direction);

	/** Returns true if a cell was found in specified direction from a center, according desired type of breaks.
	 * Could be useful to determine can the next cell be taken.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Pathfinder Type of cells searching.
	 * @param Direction Side to find the cells.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell,Side"))
	static bool CanGetCellInDirection(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    EBmrCellDirection Direction);

	/** Returns cells in specified direction from a center, according desired type of breaks.
	 * Could be useful to get horizontal cells in left and right direction or vertical cells in forward and backward directions.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell,Side"))
	static TSet<FBmrCell> GetCellsInDirections(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    int32 SideLength,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrCellDirection")) int32 DirectionsBitmask);

	/** Returns cells that match specified actors in specified direction from a center, according desired type of breaks.
	 * If none of actors are chosen, returns matching empty cells without actors in chosen direction(s).
	 * Could be useful to determine are there any players or items on the way.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Side"))
	static TSet<FBmrCell> GetCellsInDirectionsWithActors(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    int32 SideLength,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrCellDirection")) int32 DirectionsBitmask,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns matching empty cells without actors in chosen direction(s), according desired type of breaks.
	 * Could be useful to determine are there empty cells on the way.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor,Free,Side"))
	static TSet<FBmrCell> GetEmptyCellsInDirectionsWithoutActors(
	    const FBmrCell& CenterCell,
	    EPathType Pathfinder,
	    int32 SideLength,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrCellDirection")) int32 DirectionsBitmask);

	/** Returns true if player is not able to reach specified cell by any path. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell", Keywords = "Path"))
	static bool IsIslandCell(const FBmrCell& Cell);

	/** Gets a copy of given cell rotated around the given transform to the same yaw degree.
	 * @param Cell - The cell to rotate.
	 * @param AxisZ The Z param of the axis to rotate around. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell"))
	static FBmrCell RotateCellAroundLevelOrigin(const FBmrCell& Cell, float AxisZ);

	/** Gets a copy of given cell snapped to nearest cell on the level grid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InCell", Keywords = "Grid Snap,near"))
	static FORCEINLINE FBmrCell SnapCellOnLevel(const FBmrCell& Cell) { return GetCellArrayNearest(GetAllCellsOnLevel(), Cell); }

	/** Gets nearest cell on the level grid to the given vector. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Vector", Keywords = "Grid Snap,near"))
	static FORCEINLINE FBmrCell SnapVectorOnLevel(const FVector& Vector) { return SnapCellOnLevel(Vector); }

	/** Gets actor location snapped to nearest cell on the level grid.
	 * @param Actor The actor to obtain location and snap to the grid. Is not `const` because of `BlueprintAutocast` limitation to make Drag & Drop work from Actor parameter to Cell parameter. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (BlueprintAutocast))
	static FBmrCell SnapActorOnLevel(const class AActor* Actor);

	/** Returns nearest free cell to given cell, where free means cell with no other level actors except players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell"))
	static FBmrCell GetNearestFreeCell(const FBmrCell& Cell);

	/** Returns all explosion cells on Level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Dangerous"))
	static TSet<FBmrCell> GetAllExplosionCells();

	/** Returns true if any player is able to reach all specified cells by any path.
	 * @param CellsToFind Cells to which needs to find any path.
	 * @param OptionalPathBreakers Unreachable cells, where path will stop (e.g: walls), can be empty. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "OptionalPathBreakers"))
	static bool DoesPathExistToCellsOnLevel(const TSet<FBmrCell>& CellsToFind, const TSet<FBmrCell>& OptionalPathBreakers);

	/*********************************************************************************************
	 * Corner Cell
	 ********************************************************************************************* */
public:
	/** Returns 4 corner cells of the Generated Map respecting its current size. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE TSet<FBmrCell> GetCornerCellsOnLevel() { return GetCornerCellsOnGrid(GetAllCellsOnLevel()); }

	/** Returns specified corner cell in given grid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE FBmrCell GetCellByCornerOnLevel(EBmrGridCorner CornerType) { return GetCellByCornerOnGrid(CornerType, GetAllCellsOnLevel()); }

	/** Returns true if given cell is corner cell of current level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell"))
	static FORCEINLINE bool IsCornerCellOnLevel(const FBmrCell& Cell) { return GetCornerCellsOnLevel().Contains(Cell); }

	/** Return closest corner cell to the given cell.
	 * @param CellToCheck The start position of the cell to check. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "CellToCheck"))
	static FBmrCell GetNearestCornerCellOnLevel(const FBmrCell& CellToCheck);

	/*********************************************************************************************
	 * Debug cells utilities
	 ********************************************************************************************* */
public:
	/** Remove all text renders of the Owner, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static void ClearDisplayedCells(const UObject* Owner = nullptr);

	/** Display coordinates of specified cells on the level, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DevelopmentOnly, DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "Params"))
	static void DisplayCells(UObject* Owner, const TSet<FBmrCell>& Cells, const FBmrDisplayCellsParams& Params);

	/** Display only specified cell, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DevelopmentOnly, DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "Params"))
	static void DisplayCell(UObject* Owner, const FBmrCell& Cell, const FBmrDisplayCellsParams& Params) { DisplayCells(Owner, {Cell}, Params); }

	/** Returns true if cells of specified actor type(s) can be displayed.
	 * It takes into considerations the types that are set by 'Bomber.Debug.DisplayCells' cheat or directly in the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static bool CanDisplayCells(const UObject* Owner);
};