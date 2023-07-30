// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Animation/SkeletalMeshActor.h" // AMySkeletalMeshActor
#include "Components/SkeletalMeshComponent.h" // UMySkeletalMeshComponent
#include "Kismet/BlueprintFunctionLibrary.h" // UPlayerMeshDataUtils
//---
#include "Bomber.h"
//---
#include "MySkeletalMeshComponent.generated.h"

/**
 * Determines how the character looks.
 * Contains additional data.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Bomber.PlayerMeshDataUtils.MakeCustomPlayerMeshData"))
struct BOMBER_API FCustomPlayerMeshData
{
	GENERATED_BODY()

	/** Empty data. */
	static const FCustomPlayerMeshData Empty;

	/** Default constructor. */
	FCustomPlayerMeshData() = default;

	/** Constructor that initializes the player data by specified level type.
	 * It can be done since each player is unique for each level type. */
	FCustomPlayerMeshData(ELevelType PlayerByLevelType, int32 InSkinIndex);

	/** Constructor that initializes the data directly. */
	FCustomPlayerMeshData(const class UPlayerRow& InPlayerRow, int32 InSkinIndex);

	/** The row that is used to visualize the bomber character. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<const class UPlayerRow> PlayerRow = nullptr;

	/** The index of the texture to set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 SkinIndex = 0;

	/** Returns true is data is valid. */
	FORCEINLINE bool IsValid() const { return PlayerRow != nullptr; }
};

/**
 * 	The static functions library of Custom Player Mesh Data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerMeshDataUtils final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates 'Make Cell' node with Cell  as an input parameter. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "InVector", NativeMakeFunc, Keywords = "construct build"))
	static FORCEINLINE FCustomPlayerMeshData MakeCustomPlayerMeshData(ELevelType PlayerByLevelType, int32 InSkinIndex) { return {PlayerByLevelType, InSkinIndex}; }
};

class UMySkeletalMeshComponent;

/**
 * The actor that contains the player mesh component by default.
 * Is used as mesh representation in the world, mostly in cinematics.
 */
UCLASS(Blueprintable, BlueprintType)
class AMySkeletalMeshActor final : public ASkeletalMeshActor
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

	/** Applies the specified player data by given type to the mesh. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void InitMySkeletalMesh(ELevelType PlayerByLevelType, int32 InSkinIndex);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Represents with which level current spot associated with. E.g: for Hugo character the City type has to be chosen.
	 * Can be changed in editor for an instance on the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player by Level Type"))
	ELevelType PlayerByLevelTypeInternal = ELevelType::None;

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

	/** Returns true if mesh data is set. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsInitialized() const { return PlayerMeshDataInternal.IsValid(); }

	/** Returns level type to which this mesh is associated with. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ELevelType GetAssociatedLevelType() const;

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
	FCustomPlayerMeshData PlayerMeshDataInternal = FCustomPlayerMeshData::Empty;

	/** Current level type of attached meshes.
	 * Is not transient, can be set in editor-time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes Type"))
	ELevelType AttachedMeshesTypeInternal = ELT::None;

	/** Current attached mesh components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Attached Meshes"))
	TArray<TObjectPtr<class UMeshComponent>> AttachedMeshesInternal;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when a component is registered (not loaded). */
	virtual void OnRegister() override;
};
