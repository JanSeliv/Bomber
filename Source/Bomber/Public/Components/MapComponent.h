// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Structures/Cell.h"
#include "Components/ActorComponent.h"
//---
#include "MapComponent.generated.h"

/** Typedef to allow for some nicer looking sets of map components */
typedef TSet<class UMapComponent*> FMapComponents;

/**
 * These components manage their level actors updates on the level map in case of any changes that allow to:
 * -  Free location and rotation of the level map in the editor time:
 * - Prepare in advance the level actors in the editor time:
 * Same calls and initializations for each of the level map actors
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

#if WITH_EDITORONLY_DATA  // bShouldShowRenders
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly))
	bool bShouldShowRenders = false;
#endif	//WITH_EDITORONLY_DATA bShouldShowRenders

	/** The Collision Component, is attached to an owner. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UBoxComponent* BoxCollision; //[C.DO]

	/** Owner's cell location on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (ShowOnlyInnerProperties))
	FCell Cell; //[G]

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this component's properties */
	UMapComponent();

	/** Updates a owner's state. Should be called in the owner's OnConstruction event. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnConstruction();

	/** Set specified mesh to the Owner. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMeshByRow(const class ULevelActorRow* Row, class UMeshComponent* InMeshComponent = nullptr);

	/** Set material to the mesh. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMaterial(class UMaterialInterface* Material);

	/** Returns the map component of the specified owner. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE UMapComponent* GetMapComponent(const AActor* Owner) { return Owner ? Owner->FindComponentByClass<UMapComponent>() : nullptr; }

	/**  Rerun owner's construction scripts. The temporary only editor owner will not be updated. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++")
	void RerunOwnerConstruction() const;

	/** Get the owner's data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	EActorType GetActorType() const;

	/** Get the owner's data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class ULevelActorDataAsset* GetActorDataAsset() const { return ActorDataAssetInternal; }

	/** Returns true if an owner is set by cheat manager or skills to be undestroyable in game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool IsUndestroyable() const { return bIsUndestroyableInternal; }

	/** Set true to make an owner to be undestroyable on this level. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetUndestroyable(bool bIsUndestroyable);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains exposed for designers properties for the spawned owner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Actor Data Asset"))
	class ULevelActorDataAsset* ActorDataAssetInternal; //[D]

	/** */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mesh Component"))
	class UMeshComponent* MeshComponentInternal; //[C.DO]

	/** If true the owner is undestroyable, is used by skills and cheat manager. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Undestroyable"))
	bool bIsUndestroyableInternal; //[G]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() override;

	/** Called when a component is destroyed for removing the owner from the Level Map. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

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
