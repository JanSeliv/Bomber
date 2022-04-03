// Copyright (c) Yevhenii Selivanov

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

	/** Empty data. */
	static const FCustomPlayerMeshData Empty;

	/** The row that is used to visualize the bomber character. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<const class UPlayerRow> PlayerRow = nullptr; //[G]

	/** The index of the texture to set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 SkinIndex; //[N]

	/** Returns true is data is valid. */
	FORCEINLINE bool IsValid() const { return PlayerRow != nullptr; }
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

	/** Controls what kind of collision is enabled for this body and all attached props. */
	virtual void SetCollisionEnabled(ECollisionEnabled::Type NewType) override;

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

	/** Gets all attached mesh components by specified filter class.
	 * @param OutMeshComponents Contains returned components.
	 * @param FilterClass By this class components will be filtered.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "FilterClass"))
	void GetAttachedPropsByClass(TArray<class UMeshComponent*>& OutMeshComponents, const TSubclassOf<class UMeshComponent>& FilterClass) const;

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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal; //[G]

	/** Current level type of attached meshes.
	 * Is not transient, can be set in editor-time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes Type"))
	ELevelType AttachedMeshesTypeInternal = ELT::None; //[G]

	/** Current attached mesh components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes"))
	TArray<TObjectPtr<class UMeshComponent>> AttachedMeshesInternal; //[M.IO]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when a component is registered (not loaded). */
	virtual void OnRegister() override;
};
