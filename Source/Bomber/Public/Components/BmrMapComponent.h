// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Components/ActorComponent.h"

// Bomber
#include "Structures/BmrCell.h"
#include "Structures/BmrMeshData.h"

#include "BmrMapComponent.generated.h"

enum class EBmrActorType : uint8;

/** Typedef to allow for some nicer looking sets of map components */
typedef TSet<class UBmrMapComponent*> FMapComponents;

/**
 * It is designed to standardize the handling of Level Actors on a Generated Map.
 * It encapsulates the common functionality needed by different Level Actors, including:
 * - Positioning the owning actor within the grid, so GeneratedMap manages each Level Actor in abstract way through the MapComponent.
 * - Visual representation management through mesh and material settings.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UBmrMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Returns the map component of the specified owner. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DefaultToSelf = "Owner"))
	static UBmrMapComponent* GetMapComponent(const AActor* Owner);

	/** Sets default values for this component's properties */
	UBmrMapComponent();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddedToLevel, UBmrMapComponent*, InMapComponent);

	/** Called when this level actor is reconstructed or added on the Generated Map, on both server and clients.
	 * Is used by Level Actors instead of the BeginPlay(), but also called in editor preview before game even started.
	 * In Editor on construction: AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ABmrGeneratedMap::AddToGrid() -> ThisClass::OnAdded()
	 * In build: ABmrGeneratedMap::SpawnActorByType() -> ABmrGeneratedMap::AddToGrid() -> ThisClass::OnAdded()
	 * In code, BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel) can be used to bind to this event. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnAddedToLevel OnAddedToLevel;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreRemovedFromLevel, UBmrMapComponent*, InMapComponent, UObject*, DestroyCauser);

	/** Called BEFORE unregistering, owner actor is still valid and located on the level.
	 * Is useful when some logic relies on any data which will be lost after destroy.
	 * Replication: is called on both server and clients; DestroyCauser is always nullptr on clients. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnPreRemovedFromLevel OnPreRemovedFromLevel;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostRemovedFromLevel, UBmrMapComponent*, InMapComponent, UObject*, DestroyCauser);

	/** Called AFTER destroy has happened, owner actor is not valid and no longer part of the level.
	 * Is useful to perform final cleanups, like removing references to the destroyed actor.
	 * Replication: is called on both server and clients; DestroyCauser is always nullptr on clients. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnPostRemovedFromLevel OnPostRemovedFromLevel;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorTypeChanged, UBmrMapComponent*, InMapComponent);

	/** Is called when the mesh from current Data Asset is changed for owner on the level, on both server and clients.
	 * Is useful to listen when own actor is applied all visual changes like mesh, material, etc. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnActorTypeChanged OnActorTypeChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCellChanged, UBmrMapComponent*, InMapComponent, const FBmrCell&, NewCell, const FBmrCell&, PreviousCell);

	/** Called when the cell of the owner is changed on the Generated Map, on both server and clients.
	 * When changed to any valid cell, then it means the map component was added to the level (initialized) on server or replicated on clients. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnCellChanged OnCellChanged;

	/*********************************************************************************************
	 * Cell (Location)
	 ********************************************************************************************* */
public:
	/** Returns the current cell, where owner is located on the Generated Map. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrCell& GetCell() const { return LocalCell; }

	/** Allows to change locally the cell of the owner on the Generated Map.
	 * @warning Don't set it directly, use ABmrGeneratedMap::SetNearestCell(MapComponent) instead. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "Cell"))
	void SetCell(const FBmrCell& Cell);

	/** Show current cell if owned actor type is allowed, is not available in shipping build.
	 * @param bClearPrevious - if true, then all previous cells will be cleared displayed by owned Level Actor. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DevelopmentOnly))
	void TryDisplayOwnedCell(bool bClearPrevious = false);

protected:
	/** Represents the point location of the level actor owner on the Generated Map.
	 * Is not replicated here, but in the Map Components Container which is changed by the Generated Map. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	FBmrCell LocalCell = FBmrCell::InvalidCell;

	/*********************************************************************************************
	 * Mesh
	 * Is responsible for visual representation of the owner actor.
	 ********************************************************************************************* */
public:
	/** Returns a mesh component of own level actor.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UMeshComponent* GetMeshComponent() const { return MeshComponent; }

	template <typename T>
	FORCEINLINE T* GetMeshComponent() const { return Cast<T>(GetMeshComponent()); }

	/** Sets the mesh component of the owner actor.
	 * @param NewMeshComponent - the mesh component to be set on the owner. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMeshComponent(class UMeshComponent* NewMeshComponent);

	/** Returns current mesh asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UStreamableRenderAsset* GetMesh() const;

	/** Cast-version: e.g: UStaticMesh, USkeletalMesh. */
	template <typename T>
	FORCEINLINE T* GetMesh() const { return Cast<T>(GetMesh()); }

	/** Applies given mesh on owner actor, or resets the mesh if null is passed.
	 * Is useful for rows that have more than one mesh per row, like powerups.
	 * @param NewMesh - the mesh to be set on the owner, might be null to reset the mesh, but if provided, then it's required to match with any row from the Data Asset. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetLocalMesh(class UStreamableRenderAsset* NewMesh);

	/** Overrides default material of current mesh component.
	 * @param NewMaterial - the material to be set on the mesh component. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetLocalMeshMaterial(class UMaterialInterface* NewMaterial);

	/** Returns optional data of the mesh component, which is replicated to clients.
	 * Might be None, since clients are allowed to set mesh and material locally.
	 * @warning prefer to use other getters to get mesh and material directly. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrMeshData& GetReplicatedMeshData() const { return ReplicatedMeshData; }

	/** Is replicated alternative method to assign specific mesh and material to the owner.
	 * Can be called on both server and clients, so it will replicate to all others.
	 * Is optional to call, since clients are allowed to set mesh and material locally. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "MeshData"))
	void SetReplicatedMeshData(const FBmrMeshData& MeshData);

	/** Resolves the Data Registry row by name and applies its mesh locally. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TryApplyMeshFromRow(FName RowName);

protected:
	/** Cached mesh component of the owner actor: can be static or skeletal mesh component, is created on this component registration. */
	UPROPERTY(BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UMeshComponent> MeshComponent = nullptr;

	/** Is optional to set, since clients are allowed to set mesh and material locally.
	 * If set, then it will be replicated to clients. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_MeshData", AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrMeshData ReplicatedMeshData = FBmrMeshData::Empty;

	/** Server RPC to set and apply how a level actor has to look like. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "[Bomber]", meta = (BlueprintProtected, AutoCreateRefTerm = "MeshData"))
	void ServerSetMeshData(const FBmrMeshData& MeshData);

	/** Is called on client to apply replicated mesh data. */
	UFUNCTION()
	void OnRep_MeshData();

	/*********************************************************************************************
	 * Collision
	 ********************************************************************************************* */
public:
	/** Returns the collision component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UBoxComponent* GetBoxCollisionComponent() const { return BoxCollisionComponent; }

	/** Returns current collisions data of the Box Collision Component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FCollisionResponseContainer GetCollisionResponses() const;

	/** Set new collisions data for any channel of the Box Collision Component, is allowed to call on both server and clients. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "NewResponses"))
	void SetCollisionResponses(const FCollisionResponseContainer& NewResponses);

protected:
	/** The Collision Component, is attached to an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBoxComponent> BoxCollisionComponent = nullptr;

	/*********************************************************************************************
	 * Data Asset
	 ********************************************************************************************* */
public:
	/** Get the owner's data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE class UBmrLevelActorDataAsset* GetActorDataAsset() const { return ActorDataAsset; }
	const UBmrLevelActorDataAsset& GetActorDataAssetChecked() const;

	/** Returns the type of the owner: Player, Bomb, etc. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	EBmrActorType GetActorType() const;

protected:
	/** Represents the archetype of the owner, is set automatically on spawn.
	 * Is not Transient since it's set and saved in editor before the game starts. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<const class UBmrLevelActorDataAsset> ActorDataAsset = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() override;

	/** Called when a component is destroyed for removing the owner from the Generated Map. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	friend class ABmrGeneratedMap;
	friend struct FBmrMapComponentSpec;

	/** Called when this level actor is reconstructed or added on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	bool OnAdded();

	/** Is called directly from Generated Map to broadcast OnPreRemovedFromLevel delegate and performs own logic. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPreRemoved(UObject* DestroyCauser = nullptr);

	/** Is called directly from Generated Map to broadcast OnPostRemovedFromLevel delegate and performs own logic. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostRemoved(UObject* DestroyCauser = nullptr);

	/*********************************************************************************************
	 * Debug
	 ********************************************************************************************* */
public:
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (DevelopmentOnly))
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

/** Helper macro to bind and call the function when the map component is added to level */
#define BIND_ON_ADDED_TO_LEVEL(Obj, Function)                                          \
	{                                                                                  \
		if (UBmrMapComponent* InMapComponent = UBmrMapComponent::GetMapComponent(Obj)) \
		{                                                                              \
			InMapComponent->OnAddedToLevel.AddUniqueDynamic(Obj, &Function);           \
			if (InMapComponent->GetCell().IsValid())                                   \
			{                                                                          \
				Obj->Function(InMapComponent);                                         \
			}                                                                          \
		}                                                                              \
	}