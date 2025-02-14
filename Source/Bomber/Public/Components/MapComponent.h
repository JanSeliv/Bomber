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

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddedToLevel, UMapComponent*, MapComponent);

	/** Called when this level actor is reconstructed or added on the Generated Map, on both server and clients.
	 * Is used by Level Actors instead of the BeginPlay().
	 * In Editor on construction: AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> AGeneratedMap::AddToGrid() -> ThisClass::OnAdded()
	 * In build: AGeneratedMap::SpawnActorByType() -> AGeneratedMap::AddToGrid() -> ThisClass::OnAdded() */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnAddedToLevel OnAddedToLevel;

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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Cell"))
	FCell LocalCellInternal = FCell::InvalidCell;

	/*********************************************************************************************
	 * Mesh
	 ********************************************************************************************* */
public:
	/** Returns a mesh component of own level actor.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UMeshComponent* GetMeshComponent() const { return MeshComponentInternal; }

	/** Returns current mesh asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UStreamableRenderAsset* GetMesh() const;

	/** Cast-version: e.g: UStaticMesh, USkeletalMesh. */
	template <typename T>
	FORCEINLINE T* GetMesh() const { return Cast<T>(GetMesh()); }

	/** Returns the row of the current mesh.
	 * It assumes that the mesh is set by the row from the Data Asset.
	 * Is actively used by Players (e.g: EAT::Maya is Bastet player) and for Bombs (EAT::Maya is Bastet's bomb). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const ULevelActorRow* GetMeshRow() const;

	/** Cast-version: e.g: UBombRow, UItemRow. */
	template <typename T>
	const FORCEINLINE T* GetMeshRow() const { return Cast<T>(GetMeshRow()); }

	/** Applies given mesh on owner actor, or resets the mesh if null is passed.
	 * Is useful for rows that have more than one mesh per row, like items.
	 * @param NewMesh - the mesh to be set on the owner, might be null to reset the mesh, but if provided, then it's required to match with any row from the Data Asset. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMesh(class UStreamableRenderAsset* NewMesh);

	/** Overrides default material of current mesh component.
	 * @param NewMaterial - the material to be set on the mesh component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMeshMaterial(class UMaterialInterface* NewMaterial);

protected:
	/** Mesh of an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mesh Component"))
	TObjectPtr<class UMeshComponent> MeshComponentInternal = nullptr;

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

	/** Returns the type of the owne: Player, Bomb, etc. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	EActorType GetActorType() const;

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

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	friend class AGeneratedMap;
	friend struct FMapComponentSpec;

	/** Called when this level actor is reconstructed or added on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	bool OnAdded();

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