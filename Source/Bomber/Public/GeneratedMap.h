// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
//---
#include "GeneratedMap.generated.h"

/**
 * Procedurally generated grid of cells and actors on the scene.
 * @see Access its data with UGeneratedMapDataAsset (Content/Bomber/Globals/DA_Levels).
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
	/** Specify for which level actors should show debug renders. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly, Bitmask, BitmaskEnum = "/Script/Bomber.EActorType"))
	int32 RenderActorsTypes = TO_FLAG(EAT::None);
#endif	//WITH_EDITORONLY_DATA [Editor] Renders

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelMapWantsReconstruct, const FTransform&, Transform);

	/** Called when this level map actor wants to be reconstructed.
	* Is not BlueprintCallable since has to be broadcasted by ThisClass::ConstructLevelMap(). */
	UPROPERTY(BlueprintAssignable, Category = "C++")
	FOnLevelMapWantsReconstruct OnLevelMapWantsReconstruct;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetNewLevelType, ELevelType, NewLevelType);

	/** Called when new level type is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnSetNewLevelType OnSetNewLevelType;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnyPlayerDestroyed);

	/** Called when any player or bot was exploded. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnAnyPlayerDestroyed OnAnyCharacterDestroyed;

	/** Contains outside added dangerous cells, is useful for Game Features to notify bots that some cells are not safe.
	 * @todo JanSeliv 3JBOo7L8 Remove after NewAI implementation. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TSet<FCell> AdditionalDangerousCells;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- *

	/** Sets default values for this actor's properties */
	AGeneratedMap();

	/** Returns the generated map.
	 * Is created only once, can not be destroyed and always exist in persistent level. */
	static AGeneratedMap& Get();

	/** Initialize this Level Map actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructLevelMap(const FTransform& Transform);

	/** Returns number of characters in the array. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetAlivePlayersNum() const { return PlayersNumInternal; };

	/** Get the current level type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE ELevelType GetLevelType() const { return LevelTypeInternal; }

	/** Returns the camera component of the level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UMyCameraComponent* GetCameraComponent() const { return CameraComponentInternal; }

	/** Returns the Pool Manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UPoolManager* GetPoolManager() const { return PoolManagerInternal; }

	/** Spawns a level actor on the Level Map by the specified type. Then calls AddToGrid().
	 *
	 * @param Type Which type of level actors
	 * @param Cell Actors location
	 * @return Spawned actor on the Level Map, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	AActor* SpawnActorByType(EActorType Type, const FCell& Cell);

	/** Spawns a level actor on the Level Map by the specified type. */
	template <typename T>
	static FORCEINLINE T* SpawnActorByType(EActorType Type, const FCell& Cell) { return Cast<T>(Get().SpawnActorByType(Type, Cell)); }

	/** Adding and attaching the specified Map Component to the Level
	 * @param AddedComponent The Map Component of the generated or dragged level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void AddToGrid(class UMapComponent* AddedComponent);

	/** Destroy all actors from the level on specified cells.
	 * @param Cells The set of cells for destroying the found actors.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActorsOnCells(const TSet<FCell>& Cells, UObject* DestroyCauser = nullptr);

	/** Destroy level actor by specified Map Component from the level.
	 * @param MapComponent Its owner will be destroyed.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActor(UMapComponent* MapComponent, UObject* DestroyCauser = nullptr);

	/** Finds the nearest cell pointer to the specified Map Component
	 *
	 * @param MapComponent The component whose owner is being searched
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetNearestCell(class UMapComponent* MapComponent);

	/**
	* Change level by type. Specified level will be shown, other levels will be hidden.
	* @param NewLevelType the new level to apply.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetLevelType(ELevelType NewLevelType);

	/** Returns true if specified map component has non-generated owner that is manually dragged to the scene. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsDraggedMapComponent(const class UMapComponent* MapComponent) const;

	/** Returns the cached transform. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FTransform& GetCachedTransform() const { return CachedTransformInternal; }

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Gives access for the Cheat Manager to 'cheat'. */
	friend class UMyCheatManager;

	/** Gives access for helper utilities to expand cells operations on the Level Map. */
	friend class UCellsUtilsLibrary;

	/** The blueprint background actor  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Collision Component"))
	TObjectPtr<class UChildActorComponent> CollisionComponentInternal = nullptr;

	/** Cells storage. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Grid Cells", ShowOnlyInnerProperties))
	TArray<FCell> GridCellsInternal;

	/** Map components of all level actors. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Components"))
	TArray<TObjectPtr<class UMapComponent>> MapComponentsInternal;

	/** Contains map components that were dragged to the scene. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Dragged Cells"))
	TMap<FCell, EActorType> DraggedCellsInternal;

	/** Number of characters on the Level Map. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Players Num"))
	int32 PlayersNumInternal = 0;

	/** The current level type. Affects on the meshes of each level actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_LevelType", Category = "C++", meta = (BlueprintProtected, DisplayName = "Level Type"))
	ELevelType LevelTypeInternal = ELT::First;

	/** Attached camera component. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Camera Component"))
	TObjectPtr<class UMyCameraComponent> CameraComponentInternal = nullptr;

	/** Is true when current state is Game Starting. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Game Running"))
	bool bIsGameRunningInternal = false;

	/** Contains the cached transform since the level actor does not move and is always static. */
	UPROPERTY(BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Cached Transform"))
	FTransform CachedTransformInternal = FTransform::Identity;

	/** Is used to avoid spawning and destroying level actors on every regeneration,
	 *  instead they are created once, are activated when are taken from a pool
	 *  and deactivated when are returned to a pool. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Pool Manager"))
	TObjectPtr<class UPoolManager> PoolManagerInternal = nullptr;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on a this level map actor construction, could be called multiple times.
	 * Could be listened by binding to ThisClass::OnLevelMapWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructLevelMap() -> ThisClass::OnConstructionLevelMap().
	 * @warning Do not call directly, use ThisClass::ConstructLevelMap() instead. */
	UFUNCTION()
	void OnConstructionLevelMap(const FTransform& Transform);

	/** Called right before components are initialized, only called during gameplay. */
	virtual void PreInitializeComponents() override;

	/** This is called only in the gameplay before calling begin play to generate level actors */
	virtual void PostInitializeComponents() override;

	/** Called when is explicitly being destroyed to destroy level actors, not called during level streaming or gameplay ending. */
	virtual void Destroyed() override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** The intersection of (OutCells ∩ ActorsTypesBitmask).
	 *	Is not public blueprintable since all needed ufunctions are already use this method.
	 *	@see UCellsUtilsLibrary
	 *
	 * @param InOutCells Will contain cells with actors of specified types.
	 * @param ActorsTypesBitmask Bitmask of actors types to intersect.
	 * @param bIntersectAllIfEmpty If the specified set is empty, then all non-empty cells of each actor will be iterated as a source set.
	 */
	void IntersectCellsByTypes(FCells& InOutCells, int32 ActorsTypesBitmask, bool bIntersectAllIfEmpty = true) const;

	/** Getting an array of cells by any sides from an input center cell and type of breaks.
	 *	Is not public blueprintable since all needed ufunctions are already use this method.
	 *	@see UCellsUtilsLibrary
	 *
	 * @param OutCells Will contain found cells.
	 * @param Cell The start of searching by the sides.
	 * @param Pathfinder Type of cells searching.
	 * @param SideLength Distance in number of cells from a center.
	 * @param DirectionsBitmask All sides need to iterate.
	 * @param bBreakInputCells In case, specified OutCells is not empty, these cells break lines as the Wall behavior, will not be removed from the array.
	 */
	void GetSidesCells(
		TSet<FCell>& OutCells,
		const FCell& Cell,
		EPathType Pathfinder,
		int32 SideLength,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.ECellDirection")) int32 DirectionsBitmask,
		bool bBreakInputCells = false) const;

	/**
	 * Returns true if any player is able to reach all specified cells by any any path.
	 * Is not public blueprintable since all needed ufunctions are already use this method.
	 * @see UCellsUtilsLibrary
	 *
	 * @param CellsToFind Cells to which needs to find any path.
	 * @param OptionalPathBreakers Optional value for a cells that make an island, if empty, all walls on the level will break the path.
	 */
	bool DoesPathExistToCells(const FCells& CellsToFind, const FCells& OptionalPathBreakers = FCell::EmptyCells);

	/** Spawns and fills the Grid Array values by level actors */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, CallInEditor, Category = "C++", meta = (BlueprintProtected))
	void GenerateLevelActors();

	/** Map components getter.
	 *
	 * @param OutBitmaskedComponents Will contains map components of owners having the specified types.
	 * @param ActorsTypesBitmask EActorType bitmask of actors types.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetMapComponents(
		TSet<class UMapComponent*>& OutBitmaskedComponents,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask) const;

	/** Listen game states to generate level actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Align transform and build cells.
	* @param Transform New transform of the level map. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TransformLevelMap(const FTransform& Transform);

	/** Updates current level type. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyLevelType();

	/** Is called on client to load new level. */
	UFUNCTION()
	void OnRep_LevelType();

	/** Find and add all level actors to allow the Pool Manager to handle all of them. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void InitPoolManager();

	/* ---------------------------------------------------
	 *					Editor development
	 * --------------------------------------------------- */

#if WITH_EDITOR	 // [GEditor]PostLoad();
	/** Do any object-specific cleanup required immediately after loading an object. This is not called for newly-created objects. */
	virtual void PostLoad() override;
#endif	// WITH_EDITOR [GEditor]PostLoad();

	/** The dragged version of the Add To Grid function to add the dragged actor on the level. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void AddToGridDragged(class UMapComponent* AddedComponent);

	/** The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly, AutoCreateRefTerm = "NewCell"))
	void SetNearestCellDragged(const class UMapComponent* MapComponent, const FCell& NewCell);

	/** The dragged version of the Destroy Level Actor function to hide the dragged actor from the level. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void DestroyLevelActorDragged(const class UMapComponent* MapComponent);
};
