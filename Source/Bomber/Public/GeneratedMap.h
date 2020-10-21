// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "Globals//LevelActorDataAsset.h"
#include "GameFramework/Actor.h"
//---
#include "GeneratedMap.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FLevelStreamRow
{
	GENERATED_BODY()

	/** The empty mesh row. */
	static const FLevelStreamRow Empty;

	/** Default constructor */
	FLevelStreamRow() = default;

	/** The level asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<UWorld> Level; //[D]

	/** The associated type of a level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	ELevelType LevelType = ELT::None; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level", meta = (InlineEditConditionToggle))
	bool bIsStoryLevel = false; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level", meta = (EditCondition = "bIsStoryLevel"))
	FVector2D StoryLevelSize = FVector2D::ZeroVector; //[D]
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UGeneratedMapDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UGeneratedMapDataAsset() = default;

	/** Returns copies of levels. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TArray<FLevelStreamRow> GetLevelStreamRows() const { return LevelsInternal; }

	/** Get UGeneratedMapDataAsset::TickInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE float GetTickInterval() const { return TickInternal; }

	/** Get UGeneratedMapDataAsset::WallsChanceInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetWallsChance() const { return WallsChanceInternal; }

	/** Get UGeneratedMapDataAsset::BoxesChanceInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetBoxesChance() const { return BoxesChanceInternal; }

	/** Get UGeneratedMapDataAsset::CollisionsAssetInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE TSubclassOf<AActor> GetCollisionsAsset() const { return CollisionsAssetInternal; }

	/** Get UGeneratedMapDataAsset::LockLocationOnZeroInternal.  */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsLockedOnZero() const { return LockOnZeroInternal; }

protected:
	/** Contains all used levels. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Levels", ShowOnlyInnerProperties))
	TArray<FLevelStreamRow> LevelsInternal; //[D]

	/** How ofter update actors on map. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Tick Interval", ShowOnlyInnerProperties))
	float TickInternal = 0.2F; //[D]

	/** The chance of walls generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "Walls Chance", ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "100"))
	int32 WallsChanceInternal = 35;  //[AW]

	/** The chance of boxes generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "Boxes Chance", ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "100"))
	int32 BoxesChanceInternal = 70;  //[AW]

	/** Asset that contains scalable collision. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collisions Asset", ShowOnlyInnerProperties))
	TSubclassOf<AActor> CollisionsAssetInternal; //[D]

	/** If true, the level position will be locked on the (0,0,0) location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Lock Location On Zero", ShowOnlyInnerProperties))
	bool LockOnZeroInternal = true; //[D]
};

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
	 * --------------------------------------------------- */

#if WITH_EDITORONLY_DATA  // [Editor] Renders
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly, InlineEditConditionToggle))
	bool bShouldShowRenders = false;

	/** Specify for which level actors should show debug renders */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly, EditCondition = "bShouldShowRenders", Bitmask, BitmaskEnum = "EActorType"))
	int32 RenderActorsTypes = TO_FLAG(EAT::All); //[N]
#endif	//WITH_EDITORONLY_DATA [Editor] Renders

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- *

	/** Sets default values for this actor's properties */
	AGeneratedMap();

	/** Returns number of characters in the array. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE int32 GetAlivePlayersNum() const { return PlayersNumInternal; };

	/** Get the current level type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE ELevelType GetLevelType() const { return LevelTypeInternal; }

	/** Returns the camera component of the level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UMyCameraComponent* GetCameraComponent() const { return CameraComponentInternal; }

	/** Getting an array of cells by four sides of an input center cell and type of breaks.
	 *
	 * @param OutCells Will contain found cells.
	 * @param Cell The start of searching by the sides.
	 * @param SideLength Length of each side.
	 * @param bBreakInputCells In case, specified OutCells is not empty, these cells break lines as the Wall behavior, will not be removed from the array.
	 * @param Pathfinder Type of cells searching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetSidesCells(
		TSet<struct FCell>& OutCells,
		const struct FCell& Cell,
		EPathType Pathfinder,
		int32 SideLength,
		bool bBreakInputCells = false) const;

	/** Spawns a level actor on the Level Map by the specified type. Then calls AddToGrid().
	 *
	 * @param Type Which type of level actors
	 * @param Cell Actors location
	 * @return Spawned actor on the Level Map, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	AActor* SpawnActorByType(EActorType Type, const FCell& Cell);

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
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (AdvancedDisplay = 2))
	void IntersectCellsByTypes(
		UPARAM(ref) TSet<struct FCell>& OutCells,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask,
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
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask) const
	{
		FCells NonEmptyCells;
		IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask);
		return NonEmptyCells.Contains(Cell);
	}

	/** Destroy all actors from the scene and calls RemoveMapComponent(...) function.
	 *
	 * @param Cells The set of cells for destroying the found actors.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DestroyActorsFromMap(const TSet<struct FCell>& Cells);

	/** Removes the specified map component from the MapComponents_ array without an owner destroying. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DestroyLevelActor(class UMapComponent* MapComponent);

	/** Finds the nearest cell pointer to the specified Map Component
	 *
	 * @param MapComponent The component whose owner is being searched
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++")
	void SetNearestCell(class UMapComponent* MapComponent) const;

	/**
	* Change level by type. Specified level will be shown, other levels will be hidden.
	* @param NewLevelType the new level to apply.
	*/
	UFUNCTION(BlueprintCallable, Category = "C++")
    void SetLevelType(ELevelType NewLevelType);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	friend class UMyCheatManager;

	/** The blueprint background actor  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Collision Component"))
	UChildActorComponent* CollisionComponentInternal;	 //[C.DO]

	/** Cells storage. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Grid Cells", ShowOnlyInnerProperties))
	TArray<FCell> GridCellsInternal; //[M.IO]

	/** Storage of alive players and their current locations */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Components"))
	TArray<class UMapComponent*> MapComponentsInternal;  //[M.IO]

	/** Contains map components that were dragged to the scene, store it to avoid destroying and restore its owners after each regenerating. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Dragged Components"))
	TArray<class UMapComponent*> DraggedComponentsInternal;  //[M.IO]

	/** Number of characters on the Level Map. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Players Num"))
	int32 PlayersNumInternal = 0;	//[G]

	/** The current level type. Affects on the meshes of each level actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Level Type"))
	ELevelType LevelTypeInternal = ELT::First; //[N]

	/** */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Camera Component"))
	class UMyCameraComponent* CameraComponentInternal;  //[C.DO]

	/** */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Game Running"))
	bool bIsGameRunningInternal; //[G]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called before construction when a map is loaded. */
	virtual void PostLoad() override;

	/** Called when an instance of this class is placed (in editor) or spawned. */
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetMapComponents(
		TSet<class UMapComponent*>& OutBitmaskedComponents,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask) const;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/* ---------------------------------------------------
	 *					Editor development
	 * --------------------------------------------------- */

#if WITH_EDITOR	 // [Editor] Destroyed
	/** Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending */
	virtual void Destroyed() override;
#endif	// [Editor] Destroyed
};
