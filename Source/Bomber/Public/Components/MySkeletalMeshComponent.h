// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Bomber.h"
#include "Components/SkeletalMeshComponent.h"
//---
#include "MySkeletalMeshComponent.generated.h"

/**
 * Determines how the character looks.
 * Contains additional data.
 */
USTRUCT(BlueprintType)
struct FCustomPlayerMeshData
{
	GENERATED_BODY()

	/** The row that is used to visualize the bomber character. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<class UPlayerRow> PlayerRow = nullptr; //[G]

	/** The index of the texture to set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 SkinIndex; //[N]

	/** Contains up to 8 states (attached or not) about props by bits (their indexes in the array).
	  * Ex: = 4, is mean 00000100 that the only prop mesh with index #2 is attached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	uint8 AttachedPropsBitmask = 255;
};

/**
 * The Bomber Skeletal Mesh Component.
 * Used not only with the main player logic, ...
 * but in different UI actors and widgets to visualize the player's mannequin.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UMySkeletalMeshComponent final : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UMySkeletalMeshComponent();

	/**
	* Change the SkeletalMesh that is rendered for this Component. Will re-initialize the animation tree etc.
	* Is implemented to prepare it to be ready work with UPlayerDataAsset (find player row).
	* @param NewMesh New mesh to set for this component
	* @param bReinitPose Whether we should keep current pose or reinitialize.
	*/
	virtual void SetSkeletalMesh(USkeletalMesh* NewMesh, bool bReinitPose) override;

	/**
	* Changes the material applied to an element of the mesh.
	* Is implemented to set specified material as dynamic material instance
	* @param ElementIndex The element to access the material of.
	* @param Material The material to set.
	* @return the material used by the indexed element of this mesh.
	*/
	virtual void SetMaterial(int32 ElementIndex, class UMaterialInterface* Material) override;

	/** Enables or disables gravity for the owner body and all attached meshes from the player row. */
	virtual void SetEnableGravity(bool bGravityEnabled) override;

	/** Overridable internal function to respond to changes in the visibility of the component. */
	virtual void OnVisibilityChanged() override;

	/** Returns how this mesh looks like for now.
	 * @see UMySkeletalMeshComponent::PlayerMeshDataInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FCustomPlayerMeshData& GetCustomPlayerMeshData() const { return PlayerMeshDataInternal; }

	/**
	 * Init this component by specified player data.
	 * @param CustomPlayerMeshData Data to init.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CustomPlayerMeshData"))
	void InitMySkeletalMesh(const FCustomPlayerMeshData& CustomPlayerMeshData);

	/**
	 * Attach all player props.
	 * @see FAttachedMesh UPlayerRow::PlayerProps
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AttachProps();

	/**
	 * Set the skin, specified by index, to this mesh and its attached props
	 * Some bomber characters have more than 1 diffuse, it will change a player skin if possible.
	 * @param SkinIndex The index of the texture to set.
	 * @see UPlayerRow::SkinTextures
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSkin(int32 SkinIndex);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Determines how this mesh looks like for now.
	 * Is not transient, can be set in editor-time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Custom Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal; //[G]

	/** Current level type of attached meshes.
	 * Is not transient, can be set in editor-time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes Type"))
	ELevelType AttachedMeshesTypeInternal = ELevelType::None; //[G]

	/** Current attached mesh components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes"))
	TArray<TObjectPtr<class UMeshComponent>> AttachedMeshesInternal; //[M.IO]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when a component is registered (not loaded). */
	virtual void OnRegister() override;
};
