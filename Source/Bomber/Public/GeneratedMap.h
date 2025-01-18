// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
#include "Structures/MapComponentsContainer.h"
#include "Structures/GeneratedMapSettings.h"
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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneratedLevelActors);

	/** Is useful to react on regenerating level.
	 * Is called both on server and client. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGeneratedLevelActors OnGeneratedLevelActors;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostDestroyedLevelActors, const TSet<FCell>&, DestroyedCells);

	/** Called each time when any level actors were destroyed.
	 * @warning is called only on the server as destroying level actors is an authority-only operation, use UMapComponent::OnPostRemovedFromLevel for both server and clients. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPostDestroyedLevelActors OnPostDestroyedLevelActors;

	/** Contains outside added dangerous cells, is useful for Game Features to notify bots that some cells are not safe.
	 * @todo JanSeliv 3JBOo7L8 Remove after NewAI implementation. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++")
	TSet<FCell> AdditionalDangerousCells = FCell::EmptyCells;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	AGeneratedMap();

	/** Returns the generated map, it will crash if can't be obtained.
	 * @warning shouldn't be used in Getters since it will crash on levels without Generated Map.
	 * Is created only once, can not be destroyed and always exist in persistent level. */
	static AGeneratedMap& Get(const UObject* OptionalWorldContext = nullptr);

	/** Attempts to return the generated map, nullptr otherwise.
	 * Is used in Getters to avoid crashes on levels without Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static AGeneratedMap* GetGeneratedMap(const UObject* OptionalWorldContext = nullptr);

	/** Returns the settings used for generating the map.
	 * Returns overridden if is set in the Class Defaults of the Generated Map itself, otherwise defaults from the Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FGeneratedMapSettings& GetGenerationSetting() const;

	/* Allows to change the size for generated map in runtime, it will automatically regenerate the level.
	 * Is server-only function, so it will replicate the new transform to clients.
	 * @warning to change location or rotation, just call SetActorTransform, SetLocation or SetRotation.
	 * @param LevelSize The new size where length and width have to be unpaired (odd).
	 * E.g: X:9, Y:7 - set the size of the level to 9 columns (width) and 7 rows (length). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetLevelSize(const FIntPoint& LevelSize);

	/** Returns the camera component of the level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UMyCameraComponent* GetCameraComponent() const { return CameraComponentInternal; }

	/*********************************************************************************************
	 * Spawn
	 ********************************************************************************************* */
public:
	/** Spawns a level actor on the Generated Map by the specified type. Then calls AddToGrid().
	 * @param Type Which type of level actors
	 * @param Cell Actors location
	 * @return Spawned actor on the Generated Map, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (DisplayName = "Spawn Actor by Type", AutoCreateRefTerm = "Cell"))
	void BPSpawnActorByType(EActorType Type, const FCell& Cell) { SpawnActorByType(Type, Cell, nullptr); }

	/** Code alternative function with OnSpawn callback. */
	void SpawnActorByType(EActorType Type, const FCell& Cell, const TFunction<void(AActor*)>& OnSpawned = nullptr);

	/** Spawns many level actors at once, used for level generation. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SpawnActorsByTypes(const TMap<FCell, EActorType>& ActorsToSpawn);

	/** Spawns level actor of given type by the specified pattern.
	 * Is usefull for custom level generation. E.g: spawn Walls on (2,0), (3,1), (4,2) cells.
	 * @param ActorsType All existing actors with given type will be destroyed first and then spawned on the specified positions.
	 * @param Positions Columns (X) and rows (Y) positions of the actors to spawn. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, CallInEditor, Category = "C++")
	void SpawnActorsByPattern(EActorType ActorsType, const TArray<FIntPoint>& Positions);

	/** Adding and attaching the specified Map Component to the Level.
	 * @param AddedComponent The Map Component of the generated or dragged level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void AddToGrid(UMapComponent* AddedComponent);

	/*********************************************************************************************
	 * Destroy
	 ********************************************************************************************* */
public:
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

	/** Destroys level actor by specified handle.
	 * Handle version allows to destroy actor even if it is not spawned yet, but in processing queue.
	 * @param Handle Unique ID from the Pool Manager to identify the level actor to destroy.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActorByHandle(const FPoolObjectHandle& Handle, UObject* DestroyCauser = nullptr);

	/** Destroy all level actors of given type from the level.
	 * Might be useful for regenerating the level.
	 * @param ActorsType The type of actors to destroy.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActorsByType(EActorType ActorsType, UObject* DestroyCauser = nullptr);

	/** Finds the nearest cell pointer to the specified Map Component
	 *
	 * @param MapComponent The component whose owner is being searched
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetNearestCell(UMapComponent* MapComponent);

	/** Returns true if specified map component has non-generated owner that is manually dragged to the scene. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsDraggedMapComponent(const UMapComponent* MapComponent) const;

	/** Takes transform and returns aligned copy allowed to be used as actor transform for this map.
	 * @param ActorTransform The transform to align.
	 * @return Aligned transform, where:
	 * Transform location and rotation is the center of new grid
	 * Transform scale-X is number of columns.
	 * Transform scale-Y is number of rows. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorTransform"))
	static FTransform ActorTransformToGridTransform(const FTransform& ActorTransform);

	/** Set for which level actors should show debug renders, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void SetDisplayCellsActorTypes(int32 NewValue);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Gives access for helper utilities to expand operations on the Generated Map. */
	friend class UCellsUtilsLibrary;
	friend class ULevelActorsUtilsLibrary;

	/** If toggled, custom data will be used for the level generation instead of the default one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Overide Generation Settings"))
	bool bOverrideGenerationSettingsInternal = false;

	/** Is optional settings to override the default data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Overidden Generation Settings", EditCondition = "bOverrideGenerationSettingsInternal == true", EditConditionHides))
	FGeneratedMapSettings OverriddenGenerationSettingsInternal;

	/** The blueprint background actor  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Collision Component"))
	TObjectPtr<class UChildActorComponent> CollisionComponentInternal = nullptr;

	/** Cells storage. Is separated from MapComponents array,
	 * since GridCells is changing rarely (only when the level size is changed).
	 * It means, each cell represents a tile on the level, even if there is no Map Component on it. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Replicated, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Grid Cells", ShowOnlyInnerProperties))
	TArray<FCell> GridCellsInternal = FCell::EmptyCellsArr;

	/** Map components of all level actors currently spawned on the Generated Map.
	 * Is changing during the game on explosions and on the level regeneration.
	 * Array of components is wrapped by FMapComponentsContainer.
	 * It allows to replicate array faster, as the whole and even if the number of elements remains the same. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, ReplicatedUsing = "OnRep_MapComponents", Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Components"))
	FMapComponentsContainer MapComponentsInternal;

	/** Contains map components that were dragged to the scene
	 * Is set in editor by adding and dragging actors, but can be changed during the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Dragged Cells"))
	TMap<FCell, EActorType> DraggedCellsInternal;

	/** Attached camera component. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Camera Component"))
	TObjectPtr<class UMyCameraComponent> CameraComponentInternal = nullptr;

	/** Is true when current state is Game Starting. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Replicated, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Game Running"))
	bool bIsGameRunningInternal = false;

	/** Specify for which level actors should show debug renders, is not available in shipping build. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly, DisplayName = "Display Cells Actor Types", Bitmask, BitmaskEnum = "/Script/Bomber.EActorType"))
	int32 DisplayCellsActorTypesInternal = TO_FLAG(EAT::None);

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on a this Generated Map actor construction, could be called multiple times.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::OnConstructionGeneratedMap().
	 * @warning Do not call directly. */
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
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
	bool DoesPathExistToCells(const FCells& CellsToFind, const FCells& OptionalPathBreakers = FCell::EmptyCells) const;

	/** Spawns and fills the Grid Array values by level actors */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, CallInEditor, Category = "C++", meta = (BlueprintProtected))
	void GenerateLevelActors();

	/** Listen game states to generate level actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Align transform and build cells.
	* @param Transform New transform of the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void BuildGridCells(const FTransform& Transform);

	/** Scales dragged cells according new grid if sizes are different. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ScaleDraggedCellsOnGrid(const TSet<FCell>& OriginalGrid, const TSet<FCell>& NewGrid);

	/** Is called on client to broadcast On Generated Level Actors delegate. */
	UFUNCTION()
	void OnRep_MapComponents();

	/* ---------------------------------------------------
	 *					Editor development
	 * --------------------------------------------------- */

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