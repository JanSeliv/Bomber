// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Cell.generated.h"

/** Typedef to allow for some nicer looking sets of cells. */
typedef TSet<struct FCell> FCells;

/**
 * The structure that contains a location of an one cell on a grid of the Level Map.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.CellsUtilsLibrary.MakeCell", HasNativeBreak = "Bomber.CellsUtilsLibrary.BreakCell"))
struct BOMBER_API FCell
{
	GENERATED_BODY()

	/** The zero cell (0,0,-1) */
	static const FCell ZeroCell;

	/** The length of the one cell */
	static constexpr float CellSize = 200.f;

	/** Always holds the free cell's FVector-coordinate.
	 * If it is not empty or not found, holds the last succeeded due to copy operator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FVector Location = FVector::DownVector; //[AW]

	/** Default constructor. */
	FCell() = default;

	/**
	* Initial constructor for cells filling into the array.
	* Round another FVector into this cell.
	*
	* @param Vector The other vector.
	*/
	explicit FCell(const FVector& Vector);

	/** Rotates around the center of the Level Map to the same yaw degree.
	 *
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell, the same cell otherwise
	 */
	FCell RotateAngleAxis(float AxisZ) const;

	/** Comparing with uninitialized Zero Cell. */
	FORCEINLINE bool IsZeroCell() const { return *this == ZeroCell; }

	/** Check is valid this cell. */
	FORCEINLINE bool IsValid() const { return *this != ZeroCell; }

	/** Returns how many cells are between two cells. */
	template <typename T>
	static FORCEINLINE T Distance(const FCell& C1, const FCell& C2) { return FMath::Abs<T>((C1.Location - C2.Location).Size()) / CellSize; }

	/**
	 * Compares cells for equality.
	 *
	 * @param Other The other cell being compared.
	 * @return true if the points are equal, false otherwise
	 */
	FORCEINLINE bool operator==(const FCell& Other) const { return this->Location == Other.Location; }
	FORCEINLINE bool operator!=(const FCell& Other) const { return !(*this == Other); }

	/** Addition of cells. */
	FCell& operator+=(const FCell& Other);
	friend FORCEINLINE FCell operator+(const FCell& Lhs, const FCell& Rhs) { return FCell(Lhs.Location + Rhs.Location); }

	/** Subtracts a cell from another cell. */
	FCell& operator-=(const FCell& Other);
	friend FORCEINLINE FCell operator-(const FCell& Lhs, const FCell& Rhs) { return FCell(Lhs.Location - Rhs.Location); }

	/** Vector operator to return cell location. */
	FORCEINLINE operator FVector() const { return this->Location; }

	/**
	* Creates a hash value from a FCell.
	*
	* @param Vector the cell to create a hash value for
	* @return The hash value from the components
	*/
	friend FORCEINLINE uint32 GetTypeHash(const FCell& Vector) { return GetTypeHash(Vector.Location); }
};

/**
 * 	The static functions library of cells
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UCellsUtilsLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates 'Make Cell' node with Cell  as an input parameter. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InVector", NativeMakeFunc, Keywords = "construct build"))
	static FORCEINLINE FCell MakeCell(const FVector& InVector) { return FCell(InVector); }

	/** Set the values of the cell directly. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "InVector", ScriptMethod = "SetFromVector"))
	static void Cell_SetFromVector(UPARAM(ref) FCell& InCell, const FVector& InVector) { InCell.Location = InVector; }

	/** Converts a Cell to a Vector. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InVector", DisplayName = "To Vector (Cell)", CompactNodeTitle = "->", BlueprintAutocast))
	static FORCEINLINE FCell Conv_CellToVector(const FVector& InVector) { return FCell(InVector); }

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

	/** Returns the zero cell (0,0,0) */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (ScriptConstant = "Zero", ScriptConstantHost = "Cell"))
	static const FORCEINLINE FCell& Cell_Zero() { return FCell::ZeroCell; }

	/** Returns true if cell is zero. */
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

	/** Calculate the length between two cells
	 *
	 * @param C1 The first cell
	 * @param C2 The other cell
	 * @return The distance between to cells
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "C1,C2", DisplayName = "Distance (Cell)", ScriptMethod = "Distance", Keywords = "magnitude,length"))
	static FORCEINLINE double Cell_Distance(const FCell& C1, const FCell& C2) { return FCell::Distance<double>(C1, C2); }

	/** Find the average of an set of cells */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FCell GetCellArrayAverage(const TSet<FCell>& Cells);
};
