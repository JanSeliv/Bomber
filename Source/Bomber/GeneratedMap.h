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
 * 
 */
UCLASS()
class BOMBER_API AGeneratedMap final : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeneratedMap();

	/** @ingroup path_types
	 *   
	 * @param Cell 
	 * @param SideLength 
	 * @param Pathfinder 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++")
	TSet<FCell> GetSidesCells(const FCell& Cell, int32 SideLength, EPathTypesEnum Pathfinder) const;

	// Return TSet of actor cells by types
	/**
	 *   
	 * @param Keys 
	 * @param FilterTypes 
	 * @param ExcludePlayer 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2))
	TSet<FCell> FilterCellsByTypes(const TSet<FCell>& Keys, const EActorTypeEnum& FilterTypes, const class ACharacter* ExcludePlayer) const;

	/** @defgroup actors_management
	 * @{
	 * @ingroup actor_types
	 * Spawn actor by type
	 * @param Cell 
	 * @param ActorType 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	AActor* AddActorOnMap(const FCell& Cell, const EActorTypeEnum& ActorType);

	// Blueprint-overriding AddActorOnMap, update actor by obj
	/**
	 *   
	 * @param Cell 
	 * @param UpdateActor 
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddActorOnMapByObj(const FCell& Cell, AActor* UpdateActor);

	/**
	 * Destroy all actors from array of cells
	 * @param Keys An array of actors cells to be destroyed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void DestroyActorsFromMap(const TSet<FCell>& Keys);

	/** */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPushNongeneratedToMap);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "C++")
	FPushNongeneratedToMap OnActorsUpdatedDelegate;

	//  of spawned characters
	/** 
	 * Container of unique characters
	 * @see EActorTypeEnum::Player
	 * @} 
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	TSet<const class ACharacter*> CharactersOnMap_;

protected:
	friend struct FCell;

	// Called when the game starts or when spawned
	virtual void BeginPlay() final;

	// Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) final;

	// Called when this actor is explicitly being destroyed
	virtual void Destroyed() final;

	// Create LevelMap on Scene and fill TMap
	/**
	 *   
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

	/** Storage of cells and their actors
	 *
	 * @see GenerateLevelMap()
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++", meta = (DisplayName = "Grid Array"))
	TMap<FCell, const AActor*> GeneratedMap_;

	/** 
	 * @addtogroup actor_types
	 * Type and its class as associated pairs 
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TMap<EActorTypeEnum, TSubclassOf<AActor>> TypesByClassesMap_;
};
