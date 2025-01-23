// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Components/ActorComponent.h"
//---
#include "Structures/Cell.h"
//---
#include "MapComponent.generated.h"

enum class EActorType : uint8;
enum class ELevelType : uint8;

/** Typedef to allow for some nicer looking sets of map components */
typedef TSet<class UMapComponent*> FMapComponents;

/**
 * It is designed to standardize the handling of Level Actors on a Generated Map.
 * It encapsulates the common functionality needed by different Level Actors, including:
 * - Positioning the owning actor within the grid, so GeneratedMap manages each Level Actor in abstract way through the MapComponent.
 * - Visual representation management through mesh and material settings.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Returns the map component of the specified owner. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Owner"))
	static UMapComponent* GetMapComponent(const AActor* Owner);

	/** Sets default values for this component's properties */
	UMapComponent();

	/** Rerun owner's construction. Is created to:
	 * - Bypass RerunConstructionScripts() limitation that could be called only in editor.
	 * - Let others to react on construction of this actor like editor objects that implements its behaviour by binding to UMapComponent::OnOwnerWantsReconstruct.*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructOwnerActor();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOwnerWantsReconstruct);

	/** Called when this component wants to be reconstructed on the Generated Map.
	 * Is not BlueprintCallable since has to be broadcasted by ThisClass::ConstructOwnerActor(). */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++")
	FOnOwnerWantsReconstruct OnOwnerWantsReconstruct;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreRemovedFromLevel, UMapComponent*, MapComponent, UObject*, DestroyCauser);

	/** Called right before owner actor going to remove from the Generated Map, on both server and clients.
	 * Is useful to listen for actor before it is exploded and its data being reset. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPreRemovedFromLevel OnPreRemovedFromLevel;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostRemovedFromLevel, UMapComponent*, MapComponent, UObject*, DestroyCauser);

	/** Called each time after owner actor was removed from Generated Map, on both server and clients. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPostRemovedFromLevel OnPostRemovedFromLevel;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorTypeChanged, UMapComponent*, MapComponent, const class ULevelActorRow*, NewRow, const class ULevelActorRow*, PreviousRow);

	/** Is called when the Row from current Data Asset is changed for owner on the level, on both server and clients.
	 * Is useful to listen when own actor is applied all visual changes like mesh, material, etc. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnActorTypeChanged OnActorTypeChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCellChanged, UMapComponent*, MapComponent, const FCell&, NewCell, const FCell&, PreviousCell);

	/** Called when the cell of the owner is changed on the Generated Map, on both server and clients.
	 * When changed to any valid cell, then it means the map component was added to the level (initialized) on server or replicated on clients. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnCellChanged OnCellChanged;

	/*********************************************************************************************
	 * Cell (Location)
	 ********************************************************************************************* */
public:
	/** Returns the current cell, where owner is located on the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCell& GetCell() const { return LocalCellInternal; }

	/** Allows to change locally the cell of the owner on the Generated Map.
	 * @warning Don't set it directly, use AGeneratedMap::SetNearestCell(MapComponent) instead. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	void SetCell(const FCell& Cell);

	/** Show current cell if owned actor type is allowed, is not available in shipping build.
	 * @param bClearPrevious - if true, then all previous cells will be cleared displayed by owned Level Actor. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void TryDisplayOwnedCell(bool bClearPrevious = false);

protected:
	/** Represents the point location of the level actor owner on the Generated Map.
	 * Is not replicated here, but in the Map Components Container which is changed by the Generated Map. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Replicated, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Cell"))
	FCell LocalCellInternal = FCell::InvalidCell;

	/*********************************************************************************************
	 * Mesh
	 ********************************************************************************************* */
public:
	/** Returns a mesh component of own level actor.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UMeshComponent* GetMeshComponent() const { return MeshComponentInternal; }

	/** Returns the row index of the actor in the Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetRowIndex() const { return RowIndexInternal; }

	/** Returns current mesh asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UStreamableRenderAsset* GetMesh() const;

	/** Applies given mesh on owner actor, or resets the mesh if null is passed.
	 * Is useful for rows that have more than one mesh per row, like items.
	 * @param NewMesh - the mesh to be set on the owner, might be null to reset the mesh, but if provided, then it's required to match with any row from the Data Asset. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMesh(class UStreamableRenderAsset* NewMesh);

	/** Set material to the mesh. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMaterial(class UMaterialInterface* Material);

protected:
	/** Mesh of an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mesh Component"))
	TObjectPtr<class UMeshComponent> MeshComponentInternal = nullptr;

	/** Holds index of the row in Actor Data Asset. -1 means is not set.
	 * It's assuming all meshes are assigned in the Data Asset.
	 * Is primarily used for replicated of the mesh. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_RowIndex", AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Row Index"))
	int32 RowIndexInternal = -1;

	/** Is called on client to update custom mesh if changed. */
	UFUNCTION()
	void OnRep_RowIndex(int32 PreviousRowIndex);

	/*********************************************************************************************
	 * Collision
	 ********************************************************************************************* */
public:
	/** Returns the collision component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UBoxComponent* GetBoxCollisionComponent() const { return BoxCollisionComponentInternal; }

	/** Returns current collisions data of the Box Collision Component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FCollisionResponseContainer GetCollisionResponses() const;

	/** Set new collisions data for any channel of the Box Collision Component, is allowed to call on both server and clients. */ 
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "NewResponses"))
	void SetCollisionResponses(const FCollisionResponseContainer& NewResponses);

protected:
	/** The Collision Component, is attached to an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Box Collision Component"))
	TObjectPtr<class UBoxComponent> BoxCollisionComponentInternal = nullptr;

	/*********************************************************************************************
	 * Data Asset
	 ********************************************************************************************* */
public:
	/** Get the owner's data asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class ULevelActorDataAsset* GetActorDataAsset() const { return ActorDataAssetInternal; }

	const ULevelActorDataAsset& GetActorDataAssetChecked() const;

	/** Get the owner's data asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	EActorType GetActorType() const;

	/** Returns the level type by current mesh.
	 * If often used to determine the variation of the actor across different rows from the Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ELevelType GetLevelType() const;

protected:
	/** Represents the archetype of the owner, is set automatically on spawn.
	 * Is not Transient since it's set and saved in editor before the game starts. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Actor Data Asset"))
	TObjectPtr<const class ULevelActorDataAsset> ActorDataAssetInternal = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() override;

	/** Called when a component is destroyed for removing the owner from the Generated Map. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called on an owner actor construction, could be called multiple times.
	 * Could be listened by binding to ThisClass::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> [OwnerActor]::Construct[OwnerName]() -> ThisClass::ConstructOwnerActor() -> ThisClass::OnConstructionOwnerActor().
	 * @warning Do not call directly, use ThisClass::ConstructOwnerActor() instead. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	bool OnConstructionOwnerActor();

	friend class AGeneratedMap;

	/** Is called directly from Generated Map to broadcast OnPreRemovedFromLevel delegate and performs own logic. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPreRemoved(UObject* DestroyCauser = nullptr);

	/** Is called directly from Generated Map to broadcast OnPostRemovedFromLevel delegate and performs own logic. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostRemoved(UObject* DestroyCauser = nullptr);

	/*********************************************************************************************
	 * Debug
	 ********************************************************************************************* */
public:
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly))
	bool bShouldShowRenders = false;

protected:
#if WITH_EDITOR
	/** Returns true whether this component or its owner is an editor-only object or not. */
	virtual bool IsEditorOnly() const override;

	/**
	* Destroy EditorOnly owner for the editor -game.
	* Before we register our component, save it to our transaction buffer so if "undone" it will return to an unregistered state.
	* This should prevent unwanted components hanging around when undoing a copy/paste or duplication action.
	*/
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
#endif
};