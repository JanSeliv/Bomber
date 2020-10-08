// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "LevelActorDataAsset.h"
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
class BOMBER_API UMapComponent final : public UActorComponent
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FCell Cell; //[G]

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this component's properties */
	UMapComponent();

	/**
	 *	Updates a owner's state. Should be called in the owner's OnConstruction event.
	 *	@param MeshComponent Mesh component to set a mesh to an owner.
	 *	@param ComparedMeshRowTypes Specify level and item types to find its mesh. Affects on a stylistic. Can be FLevelActorMeshRow::Empty
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "ComparedMeshRowTypes"))
	void OnComponentConstruct(UMeshComponent* MeshComponent, const FLevelActorMeshRow& ComparedMeshRowTypes);

	/** Set specified mesh to the Owner. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMesh(class UStreamableRenderAsset* MeshAsset);

	/** Returns the map component of the specified owner. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE UMapComponent* GetMapComponent(const AActor* Owner)
	{
		return Owner ? Owner->FindComponentByClass<UMapComponent>() : nullptr;
	}

	/**  Rerun owner's construction scripts. The temporary only editor owner will not be updated. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++")
	void RerunOwnerConstruction() const
	{
		if (!ensureMsgf(GetOwner(), TEXT("RerunOwnerConstruction: The map owner is not valid"))) return;
		GetOwner()->RerunConstructionScripts();
	}

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class ULevelActorDataAsset* GetActorDataAsset() const { return ActorDataAssetInternal; }

	/** Get checked Data Asset */
	template <typename T>
	FORCEINLINE const T* GetDataAssetChecked() const { return CastChecked<T>(ActorDataAssetInternal); }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE EActorType GetActorType() const { return ActorDataAssetInternal ? ActorDataAssetInternal->GetActorType() : EAT::None; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains exposed for designers properties for the spawned owner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Actor Data Asset"))
	class ULevelActorDataAsset* ActorDataAssetInternal; //[D]

	/** */
	UPROPERTY(BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mesh Component"))
	class UMeshComponent* MeshComponentInternal;//[C.DO]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() override;

	/** Called when a component is destroyed for removing the owner from the Level Map. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
};
