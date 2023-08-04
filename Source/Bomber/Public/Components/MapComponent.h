// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Components/ActorComponent.h"
//---
#include "Structures/Cell.h"
//---
#include "MapComponent.generated.h"

enum class EActorType : uint8;

/** Typedef to allow for some nicer looking sets of map components */
typedef TSet<class UMapComponent*> FMapComponents;

/**
 * These components manage their level actors updates on the Generated Map in case of any changes that allow to:
 * -  Free location and rotation of the Generated Map in the editor time:
 * - Prepare in advance the level actors in the editor time:
 * Same calls and initializations for each of the Generated Map actors
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOwnerWantsReconstruct);

	/** Called when this component wants to be reconstructed on the Generated Map.
	 * Is not BlueprintCallable since has to be broadcasted by ThisClass::ConstructOwnerActor(). */
	UPROPERTY(BlueprintAssignable, Category = "C++")
	FOnOwnerWantsReconstruct OnOwnerWantsReconstruct;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeactivatedMapComponent, UMapComponent*, MapComponent, UObject*, DestroyCauser);

	/** Called when this component is destroyed on the Generated Map, is called only on the server. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, BlueprintAuthorityOnly, Category = "C++")
	FOnDeactivatedMapComponent OnDeactivatedMapComponent;

#if WITH_EDITORONLY_DATA  // bShouldShowRenders
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly))
	bool bShouldShowRenders = false;
#endif	//WITH_EDITORONLY_DATA bShouldShowRenders

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this component's properties */
	UMapComponent();

	/** Rerun owner's construction. Is created to:
	 * - Bypass RerunConstructionScripts() limitation that could be called only in editor.
	 * - Let others to react on construction of this actor like editor objects that implements its behaviour by binding to UMapComponent::OnOwnerWantsReconstruct.*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructOwnerActor();

	/** Returns the current cell, where owner is located on the Generated Map. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCell& GetCell() const { return CellInternal; }

	/** Override current cell data, where owner is located on the Generated Map.
	 * It does not move an owner on the level, to move it call AGeneratedMap::SetNearestCell function as well. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	void SetCell(const FCell& Cell);

	/** Show current cell if owned actor type is allowed, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void TryDisplayOwnedCell();

	/** Returns the owner's Level Actor Row. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class ULevelActorRow* GetLevelActorRow() const { return LevelActorRowInternal; }

	/** Returns the owner's Level Actor Row. */
	template <typename T>
	const FORCEINLINE T* GetLevelActorRow() const { return Cast<T>(GetLevelActorRow()); }

	/** Set specified mesh to the Owner. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetLevelActorRow(const class ULevelActorRow* Row);

	/** Set material to the mesh. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMaterial(class UMaterialInterface* Material);

	/** Returns the map component of the specified owner. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Owner"))
	static UMapComponent* GetMapComponent(const AActor* Owner);

	/** Get the owner's data asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	EActorType GetActorType() const;

	/** Get the owner's data asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE class ULevelActorDataAsset* GetActorDataAsset() const { return ActorDataAssetInternal; }

	const ULevelActorDataAsset& GetActorDataAssetChecked() const;

	/** Returns true if an owner is set by cheat manager or skills to be undestroyable in game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsUndestroyable() const { return bIsUndestroyableInternal; }

	/** Set true to make an owner to be undestroyable on this level. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetUndestroyable(bool bIsUndestroyable);

	/** Returns the collision component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UBoxComponent* GetBoxCollisionComponent() const { return BoxCollisionComponentInternal; }

	/** Returns current collisions data of the Box Collision Component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCollisionResponseContainer& GetCollisionResponses() const { return CollisionResponseInternal; }

	/** Set new collisions data for any channel of the Box Collision Component. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (AutoCreateRefTerm = "NewResponses"))
	void SetCollisionResponses(const FCollisionResponseContainer& NewResponses);

	/** Is called when an owner was destroyed on the Generated Map. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnDeactivated(UObject* DestroyCauser = nullptr);

	/** Returns a mesh component of own level actor.  */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UMeshComponent* GetMeshComponent() const { return MeshComponentInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Owner's cell location on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Transient, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Cell"))
	FCell CellInternal = FCell::InvalidCell;

	/** Contains exposed for designers properties for the spawned owner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Actor Data Asset"))
	TObjectPtr<const class ULevelActorDataAsset> ActorDataAssetInternal = nullptr;

	/** Mesh of an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mesh Component"))
	TObjectPtr<class UMeshComponent> MeshComponentInternal = nullptr;

	/** Current level row of the owner. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_LevelActorRow", Category = "C++", meta = (BlueprintProtected, DisplayName = "Level Actor Row"))
	TObjectPtr<const class ULevelActorRow> LevelActorRowInternal = nullptr;

	/** If true the owner is undestroyable, is used by skills and cheat manager. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Undestroyable"))
	bool bIsUndestroyableInternal = false;

	/** The Collision Component, is attached to an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Box Collision Component"))
	TObjectPtr<class UBoxComponent> BoxCollisionComponentInternal = nullptr;

	/** Actual response type of the collision box of an actor. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_CollisionResponse", Category = "C++", meta = (BlueprintProtected, DisplayName = "Collision Response"))
	FCollisionResponseContainer CollisionResponseInternal = ECR_MAX;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() override;

	/** Called when a component is destroyed for removing the owner from the Generated Map. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Is called on an owner actor construction, could be called multiple times.
	 * Could be listened by binding to ThisClass::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> [OwnerActor]::Construct[OwnerName]() -> ThisClass::ConstructOwnerActor() -> ThisClass::OnConstructionOwnerActor().
	 * @warning Do not call directly, use ThisClass::ConstructOwnerActor() instead. */
	UFUNCTION()
	bool OnConstructionOwnerActor();

	/** Is called on client to update current level actor row. */
	UFUNCTION()
	void OnRep_LevelActorRow();

	/** Updates current collisions for the Box Collision Component. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyCollisionResponse();

	/** Is called on client to respond on changes in collision responses. */
	UFUNCTION()
	void OnRep_CollisionResponse();

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
