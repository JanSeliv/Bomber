// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Animation/SkeletalMeshActor.h" // ABmrSkeletalMeshActor
#include "Components/SkeletalMeshComponent.h" // UBmrSkeletalMeshComponent

// Bomber
#include "Bomber.h" // EBmrLevelType
#include "Structures/BmrMeshData.h"
#include "Structures/BmrPlayerTag.h"

#include "BmrSkeletalMeshComponent.generated.h"

class UBmrSkeletalMeshComponent;

/**
 * The actor that contains the player mesh component by default.
 * Is used as mesh representation in the world, mostly in cinematics.
 */
UCLASS(Blueprintable, BlueprintType)
class ABmrSkeletalMeshActor : public ASkeletalMeshActor
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Default constructor, overrides in object initializer default mesh by bomber mesh. */
	ABmrSkeletalMeshActor(const FObjectInitializer& ObjectInitializer);

	/** Returns the Skeletal Mesh of the Bomber character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	UBmrSkeletalMeshComponent* GetMeshComponent() const;
	UBmrSkeletalMeshComponent& GetMeshChecked() const;

	/** Returns the Player Tag to which this mesh is associated with. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrPlayerTag& GetPlayerTag() const { return PlayerTag; }

	/** Applies the specified player data by given type to the mesh. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InPlayerTag"))
	void InitSkeletalMesh(const FBmrPlayerTag& InPlayerTag, int32 InSkinIndex);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Represents with which player current spot associated with
	 * Can be changed in editor for an instance on the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** Is current value of last chosen skin index.
	 * Can be changed in editor for an instance on the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 SkinIndex = 0;

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
class BOMBER_API UBmrSkeletalMeshComponent final : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UBmrSkeletalMeshComponent();

	/** Controls what kind of collision is enabled for this body and all attached props. */
	virtual void SetCollisionEnabled(ECollisionEnabled::Type NewType) override;

	/** Enables or disables gravity for the owner body and all attached meshes from the player row. */
	virtual void SetEnableGravity(bool bGravityEnabled) override;

	/** Overridable internal function to respond to changes in the visibility of the component. */
	virtual void OnVisibilityChanged() override;

	/** Disables tick and visibility if inactive and vice versa. */
	virtual void SetActive(bool bNewActive, bool bReset = false) override;

	/** Is overridden to properly apply the new mesh data.
	 * @warning Owner must implement the Map Component, otherwise call InitBmrSkeletalMesh directly. */
	virtual void SetSkeletalMesh(USkeletalMesh* NewMesh, bool bReinitPose = true) override;

	/** Returns how this mesh looks like for now.
	 * @see UBmrSkeletalMeshComponent::PlayerMeshData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrMeshData& GetMeshData() const { return PlayerMeshData; }

	/**
	 * Init this component by specified player data.
	 * @param MeshData Data to init.
	 */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "MeshData"))
	void InitSkeletalMesh(const FBmrMeshData& MeshData);

	/** Returns true if mesh data is set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsInitialized() const { return PlayerMeshData.IsValid(); }

	/** Returns level type to which this mesh is associated with. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	EBmrLevelType GetAssociatedLevelType() const;

	/** Returns the Player Tag to which this mesh is associated with. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrPlayerTag& GetPlayerTag() const;

	/** Gets all attached mesh components by specified filter class.
	 * @param OutMeshComponents Contains returned components.
	 * @param FilterClass By this class components will be filtered.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "FilterClass"))
	void GetAttachedPropsByClass(TArray<class UMeshComponent*>& OutMeshComponents, const TSubclassOf<class UMeshComponent>& FilterClass) const;

	/** Attach all player props from FBmrPlayerPropRow. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void AttachProps();

	/** Destroyed all currently equipped props. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void DetachProps();

	/** Returns true when is needed to attach or detach props. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool ArePropsWantToUpdate() const;

	/** Completely clears component.
	 * Components stays valid and active after this call, but all props are detached and mesh data is reset. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void Cleanup();

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Determines how this mesh looks like for now. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrMeshData PlayerMeshData = FBmrMeshData::Empty;

	/** Current level type of attached meshes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	EBmrLevelType AttachedMeshesType = ELT::None;

	/** Current attached mesh components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<TObjectPtr<class UMeshComponent>> AttachedMeshes;

	/*********************************************************************************************
	 * Skins
	 ********************************************************************************************* */
public:
	/** Returns the total number of skins for current mesh (player row). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetSkinTexturesNum() const;

	/** Checks if a skin is available and can be applied by index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsSkinAvailable(int32 SkinIdx) const;

	/** Allows to change the availability of the skin by index.
	 * @param bMakeAvailable True to unlock, false to lock.
	 * @param SkinIdx The index of the texture to change availability.
	 * @warning Unavailable skin still might apply if call ApplySkinByIndex:
	 * its responsibility of the caller to check availability with IsSkinAvailable. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetSkinAvailable(bool bMakeAvailable, int32 SkinIdx);

	/** Returns the skin row name that is currently applied to the mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE FName GetAppliedSkinRowName() const { return PlayerMeshData.SkinRowName; }

	/** Returns the positional index of the currently applied skin among skins for the same PlayerTag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetAppliedSkinIndex() const;

	/** Set and apply new skin for current mesh by FBmrPlayerSkinRow row name. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplySkinByRowName(FName InSkinRowName);

	/** Set and apply new skin for current mesh by positional index among skins for the same PlayerTag. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplySkinByIndex(int32 SkinIndex);
};