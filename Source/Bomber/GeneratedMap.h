// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SingletonLibrary.h"

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
	 * @see GenerateLevelMap()
	 */
	AGeneratedMap();

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

	/** @ingroup actor_types
	 * The intersection of input cells and actors of the specific type on these cells
	 * (Cells ∩ Actors type)  
	 * @param Cells The cells set to intersect
	 * @param ActorsTypesBitmask EActorTypeEnum bitmask to intersect
	 * @param ExcludePlayer 
	 * @return The set that contains all cells by actor types
	 * @todo to C++ IntersectionCellsByTypes(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2))
	TSet<FCell> IntersectionCellsByTypes(
		const TSet<FCell>& Cells,
		const uint8& ActorsTypesBitmask,
		const class AMyCharacter* ExcludePlayer) const;

	/**
	 * Find the actor type by key of ActorTypesByClasses
	 * @param ActorClass Class to find
	 * @return Actor type
	 * @warning Deprecated, temporary function
	 * @todo Rewrite FindTypeByClass() to C++ IntersectionCellsByTypes() 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DeprecatedNode))
	FORCEINLINE EActorTypeEnum FindTypeByClass(const TSubclassOf<AActor>& ActorClass) const
	{
		const EActorTypeEnum* FoundedActorType = USingletonLibrary::GetSingleton()->ActorTypesByClasses.FindKey(ActorClass);
		return (FoundedActorType != nullptr ? *FoundedActorType : EActorTypeEnum::None);
	}

	/** @ingroup actor_types
	 * @defgroup actors_management Storing, adding and deleting actors from GridArray_
	 * @{
	 * Spawn specific actor by type on the given cell as child actor component of Level Map 
	 * First step of adding actors to level map
	 * @param Transform The location where the child actor will be standing on
	 * @param ActorType Type of actor that will be spawned
	 * @return Spawned child actor
	 * @see AddACtorOnMapByObj(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (AutoCreateRefTerm = "ActorType"))
	FORCEINLINE AActor* AddActorOnMap(const FTransform& Transform, const EActorTypeEnum& ActorType) const
	{
		const TSubclassOf<AActor>* ActorClass = USingletonLibrary::GetSingleton()->ActorTypesByClasses.Find(ActorType);
		return (ActorClass && GetWorld() ? GetWorld()->SpawnActor<AActor>(*ActorClass, Transform) : nullptr);
	}

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
	 * @todo to C++ DestroyActorsFromMap(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void DestroyActorsFromMap(const TSet<FCell>& Keys);

	/**
	 * Set of unique characters
	 * @see EActorTypeEnum::Player
	 * @} 
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	TSet<class AMyCharacter*> CharactersOnMap;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned
	 * @todo Generate only platform without boxes*/
	virtual void OnConstruction(const FTransform& Transform) final;

#if WITH_EDITOR
	/** @defgroup [PIE]PlayInEditor Runs only in the editor before beginning play
	 * Called when this actor is explicitly being destroyed*/
	virtual void Destroyed() override;
#endif  //WITH_EDITOR [PIE]

	/** @ingroup actors_management
	 * Actors generation
	 * @see AGeneratedMap::GridArray_
	 * @see AGeneratedMap::CharactersOnMap
	 * @see struct FCell: Makes a grid of cells
	 * @todo to C++ GenerateLevelMap(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (AutoCreateRefTerm = "ActorsTypesBitmask"))
	void GenerateLevelMap(const uint8& ActorsTypesBitmask);

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
};
