// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
//---
#include "CellsUtilsLibrary.generated.h"

/**
 * 	The static functions library of cells
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
	static FORCEINLINE FCell MakeCell(const FVector& InVector) { return FCell(InVector); }

	/** Set the values of the cell directly. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "InVector", ScriptMethod = "SetFromVector"))
	static void Cell_SetFromVector(UPARAM(ref) FCell& InCell, const FVector& InVector) { InCell.Location = InVector; }

	/** Converts a Cell to a Vector. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InVector", DisplayName = "To Vector (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FCell Conv_CellToVector(const FVector& InVector) { return FCell(InVector); }

	/** Converts a cell value to a string, in the form 'X= Y=' */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To String (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FString Conv_CellToString(const FCell& InCell) { return FVector2D(InCell.Location).ToString(); }

	/** Creates 'Break Cell' node with Vector as an output parameter. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", NativeBreakFunc))
	static const FORCEINLINE FVector& BreakCell(const FCell& InCell) { return InCell.Location; }

	/** Converts a Vector to a Cell. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell", DisplayName = "To Cell (Vector)", CompactNodeTitle = "->", BlueprintAutocast))
	static const FORCEINLINE FVector& Conv_VectorToCell(const FCell& InCell) { return InCell.Location; }

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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Forward", ScriptConstantHost = "Cell"))
	static const FORCEINLINE FCell& Cell_Forward() { return FCell::ForwardCell; }

	/** Cell backward direction constant (-1,0,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Backward", ScriptConstantHost = "Cell"))
	static const FORCEINLINE FCell& Cell_Backward() { return FCell::BackwardCell; }

	/** Cell right direction constant (0,1,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Right", ScriptConstantHost = "Cell"))
	static const FORCEINLINE FCell& Cell_Right() { return FCell::RightCell; }

	/** Cell left direction constant (0,-1,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Left", ScriptConstantHost = "Cell"))
	static const FORCEINLINE FCell& Cell_Left() { return FCell::LeftCell; }

	/** Returns the zero cell (0,0,-1) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Zero", ScriptConstantHost = "Cell"))
	static const FORCEINLINE FCell& Cell_Zero() { return FCell::ZeroCell; }

	/** Returns true if cell is zero.
	 * Some functions returns the Zero Cell, so it could be useful to check is the cell found. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", ScriptMethod = "IsZero"))
	static FORCEINLINE bool Cell_IsZero(const FCell& Cell) { return Cell.IsZeroCell(); }

	/** Rotation of the input vector around the center of the Level Map to the same yaw degree
	 *
	 * @param Cell The cell, that will be rotated
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FORCEINLINE FCell RotateCellAngleAxis(const FCell& Cell, float AxisZ) { return Cell.RotateAngleAxis(AxisZ); }

	/** Calculate the length between two cells.
	 * Could be useful to check how far two cells are far between themselves.
	 *
	 * @param C1 The first cell
	 * @param C2 The other cell
	 * @return The distance between to cells in number of cells, where each 1 unit means one cell.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "C1,C2", DisplayName = "Distance (Cell)", ScriptMethod = "Distance", Keywords = "magnitude,length"))
	static FORCEINLINE double Cell_Distance(const FCell& C1, const FCell& C2) { return FCell::Distance<double>(C1, C2); }

	/** Find the average of an set of cells */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetCellArrayAverage(const TSet<FCell>& Cells) { return FCell::GetCellArrayAverage(Cells); }

	/* ---------------------------------------------------
	*		Level Map related cell functions
	*		@see trello.com/c/b2IzcOhg
	* --------------------------------------------------- */

	/** Returns the cell by specified row and column number if exists, zero cell otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const FCell& GetCellOnLevel(int32 Row, int32 Column);

	/** Takes the cell and returns its row and column position on the level if exists, -1 otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InCell"))
	static void GetCellPositionOnLevel(const FCell& InCell, int32& OutRow, int32& OutColumn);

	/** Returns all grid cell locations on the Level Map as Set. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static TSet<FCell> GetAllCellsOnLevel();

	/** Returns all grid cell locations on the Level Map as Array. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const TArray<FCell>& GetAllCellsOnLevelAsArray();

	/** Returns the cell location of the Level Map.
	 * Could be useful to get always center cell regardless of the location of the Level Map in the world. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FCell GetCenterCellOnLevel();

	/** Returns the number of columns on the Level Map. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetCellColumnsNumOnLevel();

	/** Returns the number of rows on the Level Map */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetCellRowsNumOnLevel();

	/** Returns any cell rotation on the Level Map */
	UFUNCTION(BlueprintPure, Category = "C++")
	static float GetCellRotation();

	/** Returns all empty grid cell locations on the Level Map where non of actors are present. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "free"))
	static TSet<FCell> GetAllEmptyCellsWithoutActors();

	/** Returns all grid cell locations on the Level Map by specified actor types.
	 * If non of actors are chosen, returns all empty cells without actors. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Cell By Actor"))
	static TSet<FCell> GetAllCellsWithActors(
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Takes cells and returns only empty cells where non of actors are present.
	 * Could be useful to extract only free no actor cells with within given cells. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "free"))
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
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if specified cell is empty, so it does not have own actor.
	* Could be useful to make sure there is nothing on cell, so some actor could be spawned there. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "free"))
	static bool IsEmptyCellWithoutActor(const FCell& Cell);

	/** Returns true if a cell has an actor of specified type (or its type matches with at least one type if put more than one type).
	 * If non of actors are chosen, then returns true if specified cell is empty, so it does not have own actor.
	 * Could be useful to determine does input cell contain specific actor in itself like wall, so there is no way. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static bool IsCellHasAnyMatchingActor(
		const FCell& Cell,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if at least one cell along specified is empty, so it does not have own actor.*/
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "free"))
	static bool IsAnyCellEmptyWithoutActor(const TSet<FCell>& Cells);

	/** Returns true if at least one cell has actors of specified types.
	 * If non of actors are chosen, then returns true if at least one cell along specified is empty, so it does not have own actor.
	 * Could be useful to determine do input cells contain at least one item. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool AreCellsHaveAnyMatchingActors(
		const TSet<FCell>& Cells,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if all specified cells are empty, so don't have own actors.
	 * Could be useful to make sure there are nothing on cells, so some actors could be spawned there. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "free"))
	static bool AreAllCellsEmptyWithoutActors(const TSet<FCell>& Cells);

	/** Returns true if all cells have actors of specified types.
	 * If non of actors are chosen, then returns true if all specified cells are empty, so don't have own actors.
	 * Could be useful to make sure there only players on input cells. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool AreCellsHaveAllMatchingActors(
		const TSet<FCell>& Cells,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Returns true if specified cell is present on the Level Map.
	 * Could be useful to check is input cell valid. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static bool IsCellExistsOnLevel(const FCell& Cell);

	/** Returns true if the cell is present on the Level Map with such row and column indexes.
	 * Could be useful to check row and and column. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static bool IsCellPositionExistsOnLevel(int32 Row, int32 Column);

	/** Returns true if at least one cell is present on the Level Map.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsAnyCellExistsOnLevel(const TSet<FCell>& Cells);

	/** Returns true if all specified cells are present on the Level Map.
	 * Could be useful to determine are all input cells valid. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool AreAllCellsExistOnLevel(const TSet<FCell>& Cells);

	/** Returns cells around the center in specified radius and according desired type of breaks.
	 * Could be useful to find all possible ways around.
	 *
	 * @param CenterCell The start of searching in all directions.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell"))
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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor"))
	static TSet<FCell> GetCellsAroundWithActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 Radius,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Returns matching empty cells around without actors, according desired type of breaks.
	 * Could be useful to determine are there empty cells around.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param Radius Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor"))
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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell"))
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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell"))
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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell"))
	static TSet<FCell> GetCellsInDirections(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "ECellDirection")) int32 DirectionsBitmask);

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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor"))
	static TSet<FCell> GetCellsInDirectionsWithActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "ECellDirection")) int32 DirectionsBitmask,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Returns matching empty cells without actors in chosen direction(s), according desired type of breaks.
	 * Could be useful to determine are there empty cells on the way.
	 *
	 * @param CenterCell The start of searching in specified direction.
	 * @param SideLength Distance in number of cells from a center.
	 * @param Pathfinder Type of cells searching.
	 * @param DirectionsBitmask Side to find the cells.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CenterCell", Keywords = "Cell By Actor"))
	static TSet<FCell> GetEmptyCellsInDirectionsWithoutActors(
		const FCell& CenterCell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "ECellDirection")) int32 DirectionsBitmask);

	/** Returns true if player is not able to reach specified cell by any any path. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell", Keywords = "Path"))
	static bool IsIslandCell(const FCell& Cell);
};
