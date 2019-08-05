// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "GameFramework/Actor.h"

#include "GeneratedMap.generated.h"

/**
 * Procedurally generated grid of cells and actors on the scene
 * @see USingletonLibrary::LevelMap_ reference to this Level Map
 */
UCLASS()
class BOMBER_API AGeneratedMap final : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Sets default values for this actor's properties
	 * Fill an array with associative classes and generate the level map 
	 * @see TypesByClasses_
	 * @see GenerateLevelActors()
	 */
	AGeneratedMap();

	/** The background of the Level Map */
	UPROPERTY(BlueprintReadWrite, Category = "C++")
	class UStaticMeshComponent* BackgroundMeshComponent;

	/** The static mesh of the background. Can be changed in the editor */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class UStaticMesh* BackgroundMesh;

	/** The blueprint with the collision cage and floor  */
	UPROPERTY(BlueprintReadWrite, Category = "C++")
	UChildActorComponent* PlatformComponent;

	/** Set of unique player characters  */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	TSet<class AMyCharacter*> CharactersOnMap;

	/** @ingroup path_types
	 * Getting an array of cells by four sides of a input center cell and type of breaks
	 * @param Cell The start of searching by the sides
	 * @param SideLength Length of each side
	 * @param Pathfinder Type of cells searching
	 * @return Found cells
	 * @todo to C++ GetSidesCells(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++")
	TSet<FCell> GetSidesCells(
		const FCell& Cell,
		const int32 SideLength,
		const EPathTypesEnum Pathfinder) const;

	/** @addtogroup actor_types
	 * The intersection of input cells and actors of the specific type on these cells
	 * (Cells ∩ Actors type)  
	 * @param Cells The cells set to intersect
	 * @param ActorsTypesBitmask EActorTypeEnum bitmask to intersect
	 * @param ExcludePlayer 
	 * @return The set that contains all cells by actor types
	 * @todo to C++ IntersectionCellsByTypes(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2, AutoCreateRefTerm = "ActorsTypesBitmask"))
	TSet<FCell> IntersectionCellsByTypes(
		const TSet<FCell>& Cells,
		UPARAM(meta = (Bitmask, BitmaskEnum = EActorTypeEnum)) const int32& ActorsTypesBitmask,
		const class AMyCharacter* ExcludePlayer) const;

	/**
	 * The function that places the actor on the Level Map, attaches a non-child actor and writes this actor to the GridArray_
	 * Second step of adding actors to level map
	 * @param Cell The location where the child actor will be standing on
	 * @param UpdateActor The spawned or dragged PIE actor
	 * @see AddActorOnMap(...)
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddActorOnMapByObj(const FCell& Cell, AActor* UpdateActor);

	/** Find and remove only this input actor-value of the cell-key from the Grid Array */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void RemoveActorFromGridArray(const AActor* Actor);

	/**
	 * Destroy all actors from set of cells
	 * @param Keys The set of cells for destroying the found actors
	 * @todo to C++ DestroyActorsFromMap(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void DestroyActorsFromMap(const TSet<FCell>& Keys);

protected:
	/** The level map initializing and actors generation after all of his components have been initialized, only called during gameplay */
	virtual void PostInitializeComponents() final;

	/** Called when an instance of this class is placed (in editor) or spawned
	 * @todo Generate only platform without boxes*/
	virtual void OnConstruction(const FTransform& Transform) final;

#if WITH_EDITOR  // [PIE] Destroyed()
	/** @defgroup [PIE]PlayInEditor Runs only in the editor before beginning play
	 * Called when this actor is explicitly being destroyed*/
	virtual void Destroyed() final;
#endif  //WITH_EDITOR

	/** @addtogroup [Editor]Editor
	 * Destroy all attached level actors
	 * @param bIsEditorOnlyActors Should destroy editor-only actors that were spawned in the PIE world, otherwise will be destroyed all the level map's actors
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void DestroyAttachedActors(bool bIsEditorOnlyActors = false);

	/** @ingroup actors_management
	 * Spawns and fills the Grid Array values by level actors
	 *
	 * @see AGeneratedMap::GridArray_
	 * @see AGeneratedMap::CharactersOnMap
	 * @see struct FCell: Makes a grid of cells
	 */

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (AutoCreateRefTerm = "ActorsTypesBitmask,ActorLocationToSpawn"))
	void GenerateLevelActors();

	/** @ingroup actors_management
	 * Storage of cells and their actors
	 * @see GenerateLevelMapGe()
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++", meta = (DisplayName = "Grid Array"))
	TMap<struct FCell, const AActor*> GridArray_;

#if WITH_EDITORONLY_DATA
	/** Access to the Grid Array to create a free cell without an actor
	 * @warning PIE only
	 * @see GridArray_
	 */
	friend struct FCell;

	/** @addtogroup AI
	 * Mark updating visualization(text renders) of the bot's movements in the editor
	 * @warning Editor only
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	bool bShouldShowRenders;
#endif  //WITH_EDITORONLY_DATA [Editor]

	/** The chance of boxes generation */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 BoxesChance_ = 50;
};
