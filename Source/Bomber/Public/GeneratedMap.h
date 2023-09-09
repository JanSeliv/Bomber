// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
#include "Structures/MapComponentsContainer.h"
//---
#include "GeneratedMap.generated.h"

class UMapComponent;

/**
 * Procedurally generated grid of cells and actors on the scene.
 * @see Access its data with UGeneratedMapDataAsset (Content/Bomber/DataAssets/DA_Levels).
 */
UCLASS()
class BOMBER_API AGeneratedMap final : public AActor
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratedMapWantsReconstruct, const FTransform&, Transform);

	/** Called when this Generated Map actor wants to be reconstructed.
	* Is not BlueprintCallable since has to be broadcasted by ThisClass::ConstructGeneratedMap(). */
	UPROPERTY(BlueprintAssignable, Category = "C++")
	FOnGeneratedMapWantsReconstruct OnGeneratedMapWantsReconstruct;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetNewLevelType, ELevelType, NewLevelType);

	/** Called when new level type is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnSetNewLevelType OnSetNewLevelType;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneratedLevelActors);

	/** Is useful to react on regenerating level. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnGeneratedLevelActors OnGeneratedLevelActors;

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
	 * --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	AGeneratedMap();

	/** Returns the generated map.
	 * Is created only once, can not be destroyed and always exist in persistent level. */
	static AGeneratedMap& Get();

	/** Initialize this Generated Map actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructGeneratedMap(const FTransform& Transform);

	/** Sets the size for generated map, it will automatically regenerate the level for given size.
	 * Is authority-only function.
	 * @param LevelSize The new size where length and width have to be unpaired (odd).
	 * E.g: X:9, Y:7 - set the size of the level to 9 columns (width) and 7 rows (length). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetLevelSize(const FIntPoint& LevelSize);

	/** Returns number of characters in the array. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetAlivePlayersNum() const { return PlayersNumInternal; };

	/** Get the current level type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE ELevelType GetLevelType() const { return LevelTypeInternal; }

	/** Returns the camera component of the level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UMyCameraComponent* GetCameraComponent() const { return CameraComponentInternal; }

	/** Spawns a level actor on the Generated Map by the specified type. Then calls AddToGrid().
	 *
	 * @param Type Which type of level actors
	 * @param Cell Actors location
	 * @return Spawned actor on the Generated Map, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	AActor* SpawnActorByType(EActorType Type, const FCell& Cell);

	/** Spawns a level actor on the Generated Map by the specified type. */
	template <typename T>
	static FORCEINLINE T* SpawnActorByType(EActorType Type, const FCell& Cell) { return Cast<T>(Get().SpawnActorByType(Type, Cell)); }

	/** Adding and attaching the specified Map Component to the Level
	 * @param AddedComponent The Map Component of the generated or dragged level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void AddToGrid(UMapComponent* AddedComponent);

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
	void SetNearestCell(UMapComponent* MapComponent);

	/**
	* Change level by type. Specified level will be shown, other levels will be hidden.
	* @param NewLevelType the new level to apply.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetLevelType(ELevelType NewLevelType);

	/** Returns true if specified map component has non-generated owner that is manually dragged to the scene. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsDraggedMapComponent(const UMapComponent* MapComponent) const;

	/** Takes transform and returns aligned copy allowed to be used as actor transform for this map.
	 * @param ActorTransform The transform to align.
	 * @return Aligned transform, where:
	 * Transform location and rotation is the center of new grid
	 * Transform scale-X is number of columns.
	 * Transform scale-Y is number of rows. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FTransform ActorTransformToGridTransform(const FTransform& ActorTransform);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Gives access for the Cheat Manager to 'cheat'. */
	friend class UMyCheatManager;

	/** Gives access for helper utilities to expand cells operations on the Generated Map. */
	friend class UCellsUtilsLibrary;

	/** The blueprint background actor  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Collision Component"))
	TObjectPtr<class UChildActorComponent> CollisionComponentInternal = nullptr;

	/** Cells storage. Is separated from MapComponents array,
	 * since GridCells is changing rarely (only when the level size is changed).
	 * It means, each cell represents a tile on the level, even if there is no Map Component on it. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Grid Cells", ShowOnlyInnerProperties))
	TArray<FCell> GridCellsInternal;

	/** Map components of all level actors currently spawned on the Generated Map.
	 * Is changing during the game on explosions and on the level regeneration.
	 * Array of components is wrapped by FMapComponentsContainer.
	 * It allows to replicate array faster, as the whole and even if the number of elements remains the same. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = "OnRep_MapComponents", Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Components"))
	FMapComponentsContainer MapComponentsInternal;

	/** Contains map components that were dragged to the scene
	 * Is set in editor by adding and dragging actors, but can be changed during the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Dragged Cells"))
	TMap<FCell, EActorType> DraggedCellsInternal;

	/** Number of characters on the Generated Map. */
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

	/** Specify for which level actors should show debug renders, is not available in shipping build. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly, Bitmask, BitmaskEnum = "/Script/Bomber.EActorType"))
	int32 DisplayCellsActorTypes = TO_FLAG(EAT::None);

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on a this Generated Map actor construction, could be called multiple times.
	 * Could be listened by binding to ThisClass::OnGeneratedMapWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructGeneratedMap() -> ThisClass::OnConstructionGeneratedMap().
	 * @warning Do not call directly, use ThisClass::ConstructGeneratedMap() instead. */
	UFUNCTION()
	void OnConstructionGeneratedMap(const FTransform& Transform);

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
	void IntersectCellsByTypes(FCells& InOutCells, int32 ActorsTypesBitmask, bool bIntersectAllIfEmpty) const;

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
		int32 DirectionsBitmask,
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
		TSet<UMapComponent*>& OutBitmaskedComponents,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask) const;

	/** Listen game states to generate level actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Align transform and build cells.
	* @param Transform New transform of the Generated Map. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TransformGeneratedMap(const FTransform& Transform);

	/** Scales dragged cells according new grid if sizes are different. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ScaleDraggedCellsOnGrid(const TSet<FCell>& OriginalGrid, const TSet<FCell>& NewGrid);

	/** Updates current level type. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyLevelType();

	/** Is called on client to load new level. */
	UFUNCTION()
	void OnRep_LevelType();

	/** Is called on client to broadcast On Generated Level Actors delegate. */
	UFUNCTION()
	void OnRep_MapComponents();

	/** Internal multicast function to set new size for generated map for all instances. */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "C++", meta = (BlueprintProtected))
	void MulticastSetLevelSize(const FIntPoint& LevelSize);

	/* ---------------------------------------------------
	 *					Editor development
	 * --------------------------------------------------- */

#if WITH_EDITOR	 // [GEditor]PostLoad();
	/** Do any object-specific cleanup required immediately after loading an object. This is not called for newly-created objects. */
	virtual void PostLoad() override;
#endif	// WITH_EDITOR [GEditor]PostLoad();

	/** The dragged version of the Add To Grid function to add the dragged actor on the level. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void AddToGridDragged(UMapComponent* AddedComponent);

	/** The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly, AutoCreateRefTerm = "NewCell"))
	void SetNearestCellDragged(const UMapComponent* MapComponent, const FCell& NewCell);

	/** The dragged version of the Destroy Level Actor function to hide the dragged actor from the level. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void DestroyLevelActorDragged(const UMapComponent* MapComponent);
};
