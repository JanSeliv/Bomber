// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "GameFramework/Actor.h"

#include "GeneratedMap.generated.h"

/**
 * @defgroup path_types Receiving cells for their type of danger
 * Types of breaks during cells searching on each side
 */
UENUM(BlueprintType, meta = (Bitflags))
enum class EPathTypesEnum : uint8
{
	Explosion = 1 << 0,  ///< Break to the first EActorTypeEnum::Wall without obstacles
	Free = 1 << 1,		 ///< Break to the first EActorTypeEnum::WallWall + obstacles
	Safe = 1 << 2,		 ///< Break to the first EActorTypeEnum::WallWall + obstacles + explosions
	Secure = 1 << 3		 ///< Break to the first EActorTypeEnum::WallWall + obstacles + explosions + EActorTypeEnum::Player
};

/**
 * @defgroup actor_types Group where used types of actors
 * Types of all actors on the Level Map
 * Where Walls, Boxes and Bombs are the physical barrier for players
 */
UENUM(BlueprintType, meta = (Bitflags))
enum class EActorTypeEnum : uint8
{
	Wall = 1 << 0,   ///< An absolute static and unchangeable block throughout the game
	Box = 1 << 1,	///< A destroyable Obstacle
	Bomb = 1 << 2,   ///< A destroyable exploding Obstacle
	Item = 1 << 3,   ///< A picked element giving power-up (FPowerUp struct)
	Player = 1 << 4  ///< A character that is controlled by a person or bot
};

/**
 * Level Map Actor on Scene that generates a grid of cells, and manages the actors
 * @see USingletonLibrary::LevelMap_L reference to this Level Map
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
	 * @see GenerateLevelMap()
	 */
	AGeneratedMap();

	/** @ingroup path_types
	 * Getting an array of cells by four sides of a input center cell and type of breaks
	 * @param Cell The start of searching by the sides
	 * @param SideLength Length of each side
	 * @param Pathfinder Type of cells searching
	 * @return Found cells
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++")
	TSet<FCell> GetSidesCells(
		const FCell& Cell,
		int32 SideLength,
		EPathTypesEnum Pathfinder) const;

	/** @ingroup actor_types
	 * The intersection of input cells and actors of the specific type on these cells
	 * (Cells ∩ Actors type)  
	 * @param Keys The cells set to intersect
	 * @param FilterTypes Types of actors to intersect
	 * @param ExcludePlayer 
	 * @return The set that contains all cells by actor types
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2))
	TSet<FCell> IntersectionCellsByTypes(
		const TSet<FCell>& Keys,
		EActorTypeEnum FilterTypes,
		const class ACharacter* ExcludePlayer) const;

	/** @ingroup actor_types
	 * @defgroup actors_management Storing, adding and deleting actors from GridArray_
	 * @{
	 * Spawn specific actor by type on the given cell as child actor component of Level Map 
	 * First step of adding actors to level map
	 * @param Cell The location where the child actor will be standing on
	 * @param ActorType Type of actor that will be spawned
	 * @return Spawned child actor
	 * @see AddACtorOnMapByObj(...)
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	AActor* AddActorOnMap(const FCell& Cell, const EActorTypeEnum& ActorType);

	/**
	 * The overloaded function that places the actor on the Level Map, attaches a non-child actor and writes this actor to the GridArray_
	 * Second step of adding actors to level map
	 * @param Cell The location where the child actor will be standing on
	 * @param UpdateActor The spawned or dragged PIE actor
	 * @see AddActorOnMap(...)
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddActorOnMapByObj(const FCell& Cell, AActor* UpdateActor);

	/**
	 * Destroy all actors from set of cells
	 * @param Keys The set of cells for destroying the found actors
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void DestroyActorsFromMap(const TSet<FCell>& Keys);

	/**
	 * Container of unique characters
	 * @see EActorTypeEnum::Player
	 * @} 
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	TSet<const class ACharacter*> CharactersOnMap_;

#if WITH_EDITORONLY_DATA
	/** Access to the Grid Array to create a free cell without an actor
	 * @warning PIE only
	 * @see GridArray_
	 */
	friend struct FCell;
#endif

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/** Called when this actor is explicitly being destroyed */
	virtual void Destroyed() final;

	/** @ingroup actors_management
	 * Generating the grid of cells and the actors on it
	 * @see AGeneratedMap::GridArray_
	 * @see AGeneratedMap::CharactersOnMap_
	 * @see struct FCell: Makes a grid of cells
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void GenerateLevelMap();

	/**
	 * Find nearest location as cell on Grid Array
	 * @param Actor Target to find cell location
	 * @return The cell that was found
	 * @warning Transient function for blueprint realization
	 * @todo Rewrite to code as FCell::FCell()
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "C++", meta = (DevelopmentOnly))
	FCell GetNearestCell(const AActor* Actor);

	/** @ingroup actors_management
	 * Storage of cells and their actors
	 * @see GenerateLevelMap()
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++", meta = (DisplayName = "Grid Array"))
	TMap<FCell, const AActor*> GridArray_;

	/** @ingroup actor_types
	 * Type and its class as associated pairs 
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TMap<EActorTypeEnum, TSubclassOf<AActor>> TypesByClassesMap_;
};
