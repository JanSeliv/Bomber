﻿// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Animation/SkeletalMeshActor.h" // AMySkeletalMeshActor
#include "Components/SkeletalMeshComponent.h" // UMySkeletalMeshComponent
#include "Kismet/BlueprintFunctionLibrary.h" // UPlayerMeshDataUtils
//---
#include "Bomber.h"
#include "Structures/PlayerTag.h"
#include "Structures/CustomPlayerMeshData.h"
//---
#include "MySkeletalMeshComponent.generated.h"

/**
 * 	The static functions library of Custom Player Mesh Data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerMeshDataUtils final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates 'Make Cell' node with Cell  as an input parameter. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InPlayerTag", NativeMakeFunc, Keywords = "construct build"))
	static FORCEINLINE FCustomPlayerMeshData MakeCustomPlayerMeshData(const FPlayerTag& InPlayerTag, int32 InSkinIndex) { return {InPlayerTag, InSkinIndex}; }
};

class UMySkeletalMeshComponent;

/**
 * The actor that contains the player mesh component by default.
 * Is used as mesh representation in the world, mostly in cinematics.
 */
UCLASS(Blueprintable, BlueprintType)
class AMySkeletalMeshActor : public ASkeletalMeshActor
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Default constructor, overrides in object initializer default mesh by bomber mesh. */
	AMySkeletalMeshActor(const FObjectInitializer& ObjectInitializer);

	/** Returns the Skeletal Mesh of the Bomber character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const;
	UMySkeletalMeshComponent& GetMeshChecked() const;

	/** Returns the Player Tag to which this mesh is associated with. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FPlayerTag& GetPlayerTag() const { return PlayerTagInternal; }

	/** Applies the specified player data by given type to the mesh. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "InPlayerTag"))
	void InitMySkeletalMesh(const FPlayerTag& InPlayerTag, int32 InSkinIndex);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Represents with which player current spot associated with
	 * Can be changed in editor for an instance on the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Character Tag"))
	FPlayerTag PlayerTagInternal = FPlayerTag::None;

	/** Is current value of last chosen skin index.
	 * Can be changed in editor for an instance on the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Skin Index"))
	int32 SkinIndexInternal = 0;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called right before components are initialized, only called during gameplay. */
	virtual void PreInitializeComponents() override;
};

/**
 * The Bomber Skeletal Mesh Component.
 * Used not only with the main player logic, ...
 * but in different UI actors and widgets to visualize the player's mannequin.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMySkeletalMeshComponent final : public USkeletalMeshComponent
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

	/** Disables tick and visibility if inactive and vice versa. */
	virtual void SetActive(bool bNewActive, bool bReset = false) override;

	/** Returns how this mesh looks like for now.
	 * @see UMySkeletalMeshComponent::PlayerMeshDataInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCustomPlayerMeshData& GetCustomPlayerMeshData() const { return PlayerMeshDataInternal; }

	/**
	 * Init this component by specified player data.
	 * @param CustomPlayerMeshData Data to init.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CustomPlayerMeshData"))
	void InitMySkeletalMesh(const FCustomPlayerMeshData& CustomPlayerMeshData);

	/** Creates dynamic material instance for each skin if is not done before. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateSkinTextures();

	/** Returns true if mesh data is set. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsInitialized() const { return PlayerMeshDataInternal.IsValid(); }

	/** Returns level type to which this mesh is associated with. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ELevelType GetAssociatedLevelType() const;

	/** Returns the Player Tag to which this mesh is associated with. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FPlayerTag& GetPlayerTag() const;

	/** Gets all attached mesh components by specified filter class.
	 * @param OutMeshComponents Contains returned components.
	 * @param FilterClass By this class components will be filtered.
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "FilterClass"))
	void GetAttachedPropsByClass(TArray<class UMeshComponent*>& OutMeshComponents, const TSubclassOf<class UMeshComponent>& FilterClass) const;

	/**
	 * Attach all player props.
	 * @see FAttachedMesh UPlayerRow::PlayerProps
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AttachProps();

	/** Returns true when is needed to attach or detach props. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool ArePropsWantToUpdate() const;

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Determines how this mesh looks like for now. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal = FCustomPlayerMeshData::Empty;

	/** Current level type of attached meshes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes Type"))
	ELevelType AttachedMeshesTypeInternal = ELT::None;

	/** Current attached mesh components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes"))
	TArray<TObjectPtr<class UMeshComponent>> AttachedMeshesInternal;

	/*********************************************************************************************
	 * Skins
	 ********************************************************************************************* */
public:
	/** Returns the total number of skins for current mesh (player row). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	int32 GetSkinTexturesNum() const;

	/** Checks if a skin is available and can be applied by index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	bool IsSkinAvailable(int32 SkinIdx) const;

	/** Allows to change the availability of the skin by index.
	 * @param bMakeAvailable True to unlock, false to lock.
	 * @param SkinIdx The index of the texture to change availability.
	 * @warning Unavailable skin still might apply if call ApplySkinByIndex:
	 * its responsibility of the caller to check availability with IsSkinAvailable. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSkinAvailable(bool bMakeAvailable, int32 SkinIdx);

	/** Returns the skin index that is currently applied to the mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetAppliedSkinIndex() const { return PlayerMeshDataInternal.SkinIndex; }

	/** Set and apply new skin for current mesh, by index from player row.
	 * @param SkinIndex The index of the texture to set. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ApplySkinByIndex(int32 SkinIndex);
};