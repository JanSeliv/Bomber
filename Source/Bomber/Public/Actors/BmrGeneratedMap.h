// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"

// Bomber
#include "Structures/BmrCell.h"
#include "Structures/BmrGeneratedMapSettings.h"
#include "Structures/BmrMapComponentsContainer.h"

// UE
#include "AbilitySystemInterface.h"

#include "BmrGeneratedMap.generated.h"

enum class EActorType : uint8;

class UBmrMapComponent;

/**
 * Procedurally generated grid of cells and actors on the scene.
 * @see Access its data with UGeneratedMapDataAsset (Content/Bomber/DataAssets/DA_Levels).
 */
UCLASS()
class BOMBER_API ABmrGeneratedMap final : public AActor,
                                          public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneratedLevelActors);

	/** Is useful to react on regenerating level.
	 * Is called both on server and client. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnGeneratedLevelActors OnGeneratedLevelActors;

	/** Contains outside added dangerous cells, is useful for Game Features to notify bots that some cells are not safe.
	 * @todo JanSeliv 3JBOo7L8 Remove after NewAI implementation. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]")
	TSet<FBmrCell> AdditionalDangerousCells = FBmrCell::EmptyCells;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	ABmrGeneratedMap();

	/** Returns the generated map, it will crash if can't be obtained.
	 * @warning shouldn't be used in Getters since it will crash on levels without Generated Map.
	 * Is created only once, can not be destroyed and always exist in persistent level. */
	static ABmrGeneratedMap& Get(const UObject* OptionalWorldContext = nullptr);

	/** Attempts to return the generated map, nullptr otherwise.
	 * Is used in Getters to avoid crashes on levels without Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static ABmrGeneratedMap* GetGeneratedMap(const UObject* OptionalWorldContext = nullptr);

	/** Returns the settings used for generating the map.
	 * Returns overridden if is set in the Class Defaults of the Generated Map itself, otherwise defaults from the Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrGeneratedMapSettings& GetGenerationSetting() const;

	/** Allows to override the default settings used for generating the map.
	 * Is useful for mods, game features and cheats to change the level generation settings in runtime.
	 * @param bEnableOverride If true, the InSettings will be used instead of the default ones from Data Asset.
	 * @param InSettings The new settings to use for generating the map, if bEnableOverride is true.
	 * @warning It will not regenerate the level automatically, call GenerateLevelActors() manually or restart the game (change the game state). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InSettings"))
	void SetOverriddenGenerationSettings(bool bEnableOverride, const FBmrGeneratedMapSettings& InSettings);

	/* Allows to change the size for generated map in runtime, it will automatically regenerate the level.
	 * Is server-only function, so it will replicate the new transform to clients.
	 * @warning to change location or rotation, just call SetActorTransform, SetLocation or SetRotation.
	 * @param LevelSize The new size where length and width have to be unpaired (odd).
	 * E.g: X:9, Y:7 - set the size of the level to 9 columns (width) and 7 rows (length). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetLevelSize(const FIntPoint& LevelSize);

	/** Returns the camera component of the level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UBmrCameraComponent* GetCameraComponent() const { return CameraComponent; }

	/*********************************************************************************************
	 * Spawn
	 ********************************************************************************************* */
public:
	/** Spawns a level actor on the Generated Map by the specified type. Then calls AddToGrid().
	 * @param Type Which type of level actors
	 * @param Cell Actors location
	 * @return Spawned actor on the Generated Map, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DisplayName = "Spawn Actor by Type", AutoCreateRefTerm = "Cell"))
	void K2_SpawnActorByType(EBmrActorType Type, const FBmrCell& Cell) { SpawnActorByType(Type, Cell, nullptr); }

	/** Code alternative function with OnSpawn callback. */
	void SpawnActorByType(EBmrActorType Type, const FBmrCell& Cell, const TFunction<void(UBmrMapComponent&)>& OnSpawned = nullptr);

	/** Blueprint-only method to spawn multiple level actors at once. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DisplayName = "Spawn Actors by Types"))
	void K2_SpawnActorsByTypes(const TMap<FBmrCell, EBmrActorType>& ActorsToSpawn) { SpawnActorsByTypes(ActorsToSpawn, nullptr); }

	/** Code alternative function with OnSpawn callback, is mostly used for level generation. */
	void SpawnActorsByTypes(const TMap<FBmrCell, EBmrActorType>& ActorsToSpawn, const TFunction<void(const TArray<UBmrMapComponent*>&)>& OnSpawned = nullptr);

	/** Spawns level actor of given type by the specified pattern.
	 * Is useful for custom level generation. E.g: spawn Walls on (2,0), (3,1), (4,2) cells.
	 * @param ActorsType All existing actors with given type will be destroyed first and then spawned on the specified positions.
	 * @param Positions Columns (X) and rows (Y) positions of the actors to spawn. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SpawnActorsByPattern(EBmrActorType ActorsType, const TArray<FIntPoint>& Positions);

	/** Spawns level actors with the specified mesh data on the Generated Map.
	 * @param ActorType The type of actors to spawn.
	 * @param Cell The cell to spawn the actor.
	 * @param MeshData The mesh data to apply to the spawned actor. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell,MeshData"))
	void SpawnActorWithMesh(EBmrActorType ActorType, const FBmrCell& Cell, const struct FBmrMeshData& MeshData);

	/** Adding and attaching the specified Map Component to the Level.
	 * @param AddedComponent The Map Component of the generated or dragged level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void AddToGrid(UBmrMapComponent* AddedComponent);

	/** Internal client-only method to resolve a newly spawned Map Component.
	 * Unlike actors, components lack automatic FNetGUID resolution if the reference replicates
	 * before the component is spawned. Manually binds the component to its replicated entry. */
	void ResolveSpawnedMapComponent(UBmrMapComponent& AddedComponent);

	/** Internal method called on both server and client to increment the replication token whenever any level actor is spawned. */
	void IncrementReplicationToken();

	/*********************************************************************************************
	 * Destroy
	 ********************************************************************************************* */
public:
	/** Destroy all actors from the level on specified cells.
	 * @param Cells The set of cells for destroying the found actors.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActorsOnCells(const TSet<FBmrCell>& Cells, UObject* DestroyCauser = nullptr);

	/** Destroy level actor by specified Map Component from the level.
	 * @param MapComponent Its owner will be destroyed.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActor(UBmrMapComponent* MapComponent, UObject* DestroyCauser = nullptr);

	/** Destroys level actor by specified handle.
	 * Handle version allows to destroy actor even if it is not spawned yet, but in processing queue.
	 * @param Handle Unique ID from the Pool Manager to identify the level actor to destroy.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActorByHandle(const FPoolObjectHandle& Handle, UObject* DestroyCauser = nullptr);

	/** Destroy all level actors of given type from the level.
	 * Might be useful for regenerating the level.
	 * @param ActorsType The type of actors to destroy.
	 * @param DestroyCauser The actor that caused the destruction of the level actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DefaultToSelf = "DestroyCauser"))
	void DestroyLevelActorsByType(EBmrActorType ActorsType, UObject* DestroyCauser = nullptr);

	/** Is called before Destroy happening, which only unregisters the Map Component.
	 * It does not destroy its owner actor yet, so it wil remain on the scene as not interactable object until fully destroyed.
	 * Is part of DestroyLevelActor() flow, but might be useful to call directly in cases when need to keep actor on level disabled (e.g: dead player). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (DefaultToSelf = "Causer"))
	void RemoveFromGrid(UBmrMapComponent* MapComponent, UObject* RemoveCauser = nullptr);

	/** Applies the snapped cell to the specified Map Component.
	 * @param InMapComponent The Map Component to apply the snapped cell.
	 * @return true if changed: new cell was applied to given Map Component; false is already snapped or failed. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	bool SetNearestCell(UBmrMapComponent* InMapComponent);

	/** Takes transform and returns aligned copy allowed to be used as actor transform for this map.
	 * @param ActorTransform The transform to align.
	 * @return Aligned transform, where:
	 * Transform location and rotation is the center of new grid
	 * Transform scale-X is number of columns.
	 * Transform scale-Y is number of rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "ActorTransform"))
	static FTransform ActorTransformToGridTransform(const FTransform& ActorTransform);

	/*********************************************************************************************
	 * Gameplay Ability System (GAS)
	 ********************************************************************************************* */
public:
	/** Returns ability system component that is used to manage environmental abilities.
	 * In blueprints, can be obtained via regular Get Ability System Component call from ability interface. */
	virtual FORCEINLINE UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UAbilitySystemComponent& GetAbilitySystemComponentChecked() const;

protected:
	/** Ability System Component that is used to manage abilities (like place bomb) and attributes (like powerups) for owned player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "[Bomber]", meta = (BlueprintProtected, DisplayName = "Ability System Component"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	/** Attribute set for damage (e.g: environmental explosions) and health (e.g: damaging the level).
	 * For read access, can be obtained with UBmrHealthAttributeSet::GetHealthAttributeSet(Owner) method.
	 * For write access, apply gameplay effects. */
	UPROPERTY(Transient)
	TObjectPtr<class UBmrHealthAttributeSet> HealthSet = nullptr;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Gives access for helper utilities to expand operations on the Generated Map. */
	friend class UBmrCellUtilsLibrary;
	friend class UBmrActorUtilsLibrary;

	/** If toggled, custom data will be used for the level generation instead of the default ones from Data Asset.
	 * Can be set right in the Details Panel of the Generated Map actor on the scene, individually per each level.
	 * Is also useful for mods, game features and cheats. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bOverrideGenerationSettings = false;

	/** Is optional settings to override the default data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected, EditCondition = "bOverrideGenerationSettings == true", EditConditionHides))
	FBmrGeneratedMapSettings OverriddenGenerationSettings = FBmrGeneratedMapSettings::Empty;

	/** The blueprint background actor  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UChildActorComponent> CollisionComponent = nullptr;

	/** Cells storage. Is separated from MapComponents array,
	 * since GridCells is changing rarely (only when the level size is changed).
	 * It means, each cell represents a tile on the level, even if there is no Map Component on it.
	 * Is not replicated and building locally on each instance based on replicated actor transform of the Generated Map. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Transient, Category = "[Bomber]", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<FBmrCell> LocalGridCells = FBmrCell::EmptyCellsArr;

	/** Stores and replicates map components of currently spawned level actors.
	 * Updates dynamically during level regeneration, explosions, player movement, and item spawns.
	 * Uses a fast array to replicate efficiently while minimizing network usage.
	 * Ensures reliable replication even when the number of components remains unchanged. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Replicated, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrMapComponentsContainer MapComponents;

	/** Is exposed to track when the Generate Level Actors is completed on the client.
	 * Server updates this token on every Generate Level Actors call.
	 * Client monitors updates and confirms when tokens match to broadcast the On Generated Level Actors event. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Replicated, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 GenerateLevelActorsToken = 0;

	/** Is server-only, true while the level actors are being generated, is useful to avoid reentry.
	 * It takes some time to process as firstly it starts async task to compute pattern, and then spawns actors spreading over multiple frames.
	 * Once completed, it will broadcast the OnGeneratedLevelActors event on both server and clients. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bIsCurrentlyGenerating = false;

	/** Attached camera component. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrCameraComponent> CameraComponent = nullptr;

	/** Specify for which level actors should show debug renders, is not available in shipping build.
	 * Can be overridden by `Bomber.Debug.DisplayCells VALUES` cheat. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "[Bomber]", meta = (DevelopmentOnly, Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType"))
	int32 DisplayCellsActorTypes = 0;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	friend class UBmrCheatManager;

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on this Generated Map actor construction or when level size (transform) is changed, on both server and clients.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::OnConstructionGeneratedMap().
	 * @warning Do not call directly, but change the actor transform instead. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnConstructionGeneratedMap(const FTransform& Transform);

	/** Called right before components are initialized, only called during gameplay. */
	virtual void PreInitializeComponents() override;

	/** This is called only in the gameplay before calling begin play to generate level actors */
	virtual void PostInitializeComponents() override;

	/** Called when is explicitly being destroyed to destroy level actors, not called during level streaming or gameplay ending. */
	virtual void Destroyed() override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Spawns and fills the Grid Array values by level actors */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, CallInEditor, Category = "[Bomber]", meta = (BlueprintProtected))
	void GenerateLevelActors();

	/** Internal methods to compute cells on background thread and finish with spawning on the game thread. */
	static TMap<FBmrCell, EBmrActorType> GenerateLevelActors_StartAsync(struct FBmrGeneratorData&& GeneratorData);
	void GenerateLevelActors_Finish(TMap<FBmrCell, EBmrActorType>&& ActorsToSpawn);

	/** Listen game states to generate level actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(EBmrCurrentGameState CurrentGameState);

	/** Align transform and build cells, on both server and clients.
	 * Is called everytime the level size (transform) is changed.
	 * @param Transform New transform of the Generated Map. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void BuildGridCells(const FTransform& Transform);

	/*********************************************************************************************
	 * Dragged Level Actors
	 ********************************************************************************************* */
public:
	/** Returns true if specified map component has non-generated owner that is manually dragged to the scene. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsDraggedMapComponent(const UBmrMapComponent* MapComponent) const;

	/** Scales dragged cells according new grid if sizes are different. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ScaleDraggedCellsOnGrid(const TSet<FBmrCell>& OriginalGrid, const TSet<FBmrCell>& NewGrid);

	/** The dragged version of the Add To Grid function to add the dragged actor on the level. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected, DevelopmentOnly))
	void AddToGridDragged(UBmrMapComponent* AddedComponent);

	/** The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor.
	 * @param MapComponent The Map Component of the dragged level actor.
	 * @param InOutCell Takes suggested cell and performs additional snaps; or the same cell if not dragged.
	 * @return true if the cell handled dragged actor, false otherwise. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected, DevelopmentOnly))
	bool SetNearestCellDragged(const UBmrMapComponent* MapComponent, UPARAM(ref) FBmrCell& InOutCell);

	/** The dragged version of the Destroy Level Actor function to hide the dragged actor from the level. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected, DevelopmentOnly))
	void DestroyLevelActorDragged(const UBmrMapComponent* MapComponent);

protected:
	/** Contains map components that were dragged to the scene
	 * Is set in editor by adding and dragging actors, but can be changed during the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TMap<FBmrCell, EBmrActorType> DraggedCells;
};