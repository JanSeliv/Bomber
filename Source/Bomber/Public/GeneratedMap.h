// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "GameFramework/Actor.h"

#include "GeneratedMap.generated.h"

/**
 * Procedurally generated grid of cells and actors on the scene.
 * @see USingletonLibrary::LevelMap_ reference to this Level Map.
 */
UCLASS()
class BOMBER_API AGeneratedMap final : public AActor
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- *
	/** The blueprint background actor  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	UChildActorComponent* BackgroundBlueprintComponent;  //[C.DO]

	/** The blueprint class with the background, collision cage and floor. Can be changed in the editor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TSubclassOf<AActor> BackgroundBlueprintClass;  //[B]

#if WITH_EDITORONLY_DATA  // bShouldShowRenders
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly))
	bool bShouldShowRenders = false;
#endif  //WITH_EDITORONLY_DATA [Editor] bShouldShowRenders

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- *

	/** Sets default values for this actor's properties */
	AGeneratedMap();

	/** Getting an array of cells by four sides of an input center cell and type of breaks.
	 *
	 * @param OutCells Will contain found cells. 
	 * @param Cell The start of searching by the sides.
	 * @param SideLength Length of each side.
	 * @param bBreakInputCells In case, specified OutCells is not empty, these cells break lines as the Wall behavior, will not be removed from the array.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Pathfinder, SideLength"))
	void GetSidesCells(
		TSet<struct FCell>& OutCells,
		const struct FCell& Cell,
		const EPathType& Pathfinder,
		const int32& SideLength,
		bool bBreakInputCells = false) const;

	/** Spawns a level actor on the Level Map by the specified type. Then calls AddToGrid().
	 * 
	 * @param Type Which type of level actors
	 * @param Cell Actors location
	 * @return Spawned actor on the Level Map, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Type,Cell"))
	AActor* SpawnActorByType(const EActorType& Type, const FCell& Cell);

	/** Adding and attaching the specified Map Component to the MapComponents_ array

	 * @param Cell The location where the owner will be standing on
	 * @param AddedComponent The Map Component of the generated or dragged level actor
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddToGrid(const struct FCell& Cell, class UMapComponent* AddedComponent);

	/** The intersection of (OutCells ∩ ActorsTypesBitmask).
	 * 
	 * @param OutCells Will contains cells with actors of specified types.
	 * @param ActorsTypesBitmask Bitmask of actors types to intersect.
	 * @param bIntersectAllIfEmpty If the specified set is empty, then all non-empty cells of each actor will be iterated as a source set.
	 * @param ExceptedComponent Is not included for intersecting.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (AdvancedDisplay = 2, AutoCreateRefTerm = "ActorsTypesBitmask"))
	void IntersectCellsByTypes(
		UPARAM(ref) TSet<struct FCell>& OutCells,
		UPARAM(meta = (Bitmask, BitmaskEnum = EActorType)) const int32& ActorsTypesBitmask,
		bool bIntersectAllIfEmpty = true,
		const class UMapComponent* ExceptedComponent = nullptr) const;

	/** Checking the containing of the specified cell among owners locations of MapComponents_ array.
	 *
	 * @param Cell The cell to check.
	 * @param ActorsTypesBitmask Bitmask of actors types to check.
	 * @return true if at least one level actor is contained.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool ContainsMapComponents(
		const struct FCell& Cell,
		UPARAM(meta = (Bitmask, BitmaskEnum = EActorType)) const int32& ActorsTypesBitmask) const
	{
		FCells NonEmptyCells;
		IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask);
		return NonEmptyCells.Contains(Cell);
	}

	/** Destroy all actors from the scene and calls RemoveMapComponent(...) function.
	 * 
	 * @param Cells The set of cells for destroying the found actors.
	 * @param bIsNotValidOnly If should destroy editor-only actors that were spawned in the PIE world
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DestroyActorsFromMap(const TSet<struct FCell>& Cells, bool bIsNotValidOnly = false);

	/** Removes the specified map component from the MapComponents_ array without an owner destroying. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void RemoveMapComponent(class UMapComponent* MapComponent);

	/** Finds the nearest cell pointer to the specified Map Component 
	 * 
	 * @param MapComponent The component whose owner is being searched
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++")
	void SetNearestCell(class UMapComponent* MapComponent) const;

	/** Returns number of characters in the array. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetCharactersNum() const { return PlayerCharactersNum; }

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- *

	/** Cells storage. */
	TArray<FSharedCell> GridCells_;  //[M.IO]

	/** Storage of alive players and their current locations */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected))
	TArray<class UMapComponent*> MapComponents_;  //[M.IO]

	/** The chance of walls generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, ClampMin = "0", ClampMax = "100"))
	int32 WallsChance_ = 35;  //[AW]

	/** The chance of boxes generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, ClampMin = "0", ClampMax = "100"))
	int32 BoxesChance_ = 70;  //[AW]

	/** Number of characters on the Level Map. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected))
	int32 PlayerCharactersNum = 0;  //[G]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called every time on this actor to update characters locations */
	virtual void Tick(float DeltaTime) override;

	/** Called when an instance of this class is placed (in editor) or spawned
	 * @todo Generate only platform without boxes*/
	virtual void OnConstruction(const FTransform& Transform) override;

	/** This is called only in the gameplay before calling begin play to generate level actors */
	virtual void PostInitializeComponents() override;

	/** Spawns and fills the Grid Array values by level actors */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "C++", meta = (BlueprintProtected))
	void GenerateLevelActors();

	/** Map components getter.
	 *
	 * @param OutBitmaskedComponents Will contains map components of owners having the specified types.
	 * @param ActorsTypesBitmask EActorType bitmask of actors types.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ActorsTypesBitmask"))
	void GetMapComponents(
		TSet<class UMapComponent*>& OutBitmaskedComponents,
		UPARAM(meta = (Bitmask, BitmaskEnum = EActorType)) const int32& ActorsTypesBitmask) const;
	/* ---------------------------------------------------
	 *					Editor development
	 * --------------------------------------------------- */

#if WITH_EDITOR  // [Editor] Destroyed
	/** Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending */
	virtual void Destroyed() override;
#endif  // [Editor] Destroyed
};
