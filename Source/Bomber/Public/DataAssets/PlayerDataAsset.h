// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"
//---
#include "Structures/PlayerTag.h"
//---
#include "PlayerDataAsset.generated.h"

/**
 * Determines each mesh to attach.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FAttachedMesh
{
	GENERATED_BODY()

	/** The attached static mesh or skeletal mesh.  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties, ExposeOnSpawn))
	TObjectPtr<const class UStreamableRenderAsset> AttachedMesh = nullptr;

	/** In the which socket should attach this prop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FName Socket = NAME_None;

	/** Prop animation is loop played all the time, starts playing on attaching to the owner. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimSequence> MeshAnimation = nullptr;
};

/**
 * The player archetype of level actor rows. Determines the individual of the character model
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerRow final : public ULevelActorRow
{
	GENERATED_BODY()

public:
	/** The tag of this player character to be used for association of this player with other data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Row", meta = (ShowOnlyInnerProperties))
	FPlayerTag PlayerTag = FPlayerTag::None;

	/** All meshes that will be attached to the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TArray<FAttachedMesh> PlayerProps;

	/** The own movement animation for the each character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UBlendSpace1D> IdleWalkRunBlendSpace = nullptr;

	/** Dance animation that is used mostly in the menu instead of idle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimSequence> DanceAnimation = nullptr;

	/** Returns the num of skin textures in the array of diffuse maps specified a player material instance. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetMaterialInstancesDynamicNum() const { return MaterialInstancesDynamicInternal.Num(); }

	/** Returns the dynamic material instance of a player with specified skin.
	 * @param SkinIndex The skin position to get.
	 * @see UPlayerRow::MaterialInstancesDynamicInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMaterialInstanceDynamic* GetMaterialInstanceDynamic(int32 SkinIndex) const;

protected:
	/** The material instance of a player.
	 * @warning Is not BlueprintReadOnly and has not getter to prevent being used directly, we have dynamic materials instead.
	 * @see UPlayerRow::MaterialInstancesDynamicInternal. */
	UPROPERTY(EditDefaultsOnly, Category = "Row", meta = (BlueprintProtected, DisplayName = "Material Instance", ShowOnlyInnerProperties))
	TObjectPtr<class UMaterialInstance> MaterialInstanceInternal = nullptr;

	/**
	 * Contains all created dynamic materials for each skin in the Material Instance.
	 * Saves memory avoiding creation of dynamic materials for each mesh component, just use the same dynamic material for different meshes with the same skin.
	 * Is filled on object creating and changing.
	 * @warning Is not EditDefaultsOnly because have to be created dynamically, in the same time is incompatible with VisibleInstanceOnly.
	 * @see UPlayerRow::MaterialInstanceInternal. */
	UPROPERTY(BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Material Instances Dynamic"))
	TArray<TObjectPtr<class UMaterialInstanceDynamic>> MaterialInstancesDynamicInternal;

#if WITH_EDITOR
	/** Handle adding and changing material instance to prepare dynamic materials. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Create dynamic material instance for each ski if is not done before.
	 * UPlayerRow::MaterialInstancesDynamicInternal
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryCreateDynamicMaterials();
#endif	//WITH_EDITOR
};

/**
 * The data asset of the Bomber characters
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UPlayerDataAsset();

	/** Returns the player data asset. */
	static const UPlayerDataAsset& Get();

	/** The num of nameplate materials.  */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetNameplateMaterialsNum() const { return NameplateMaterialsInternal.Num(); }

	/** Returns a nameplate material by index, is used by nameplate meshes.
	 * @see UPlayerDataAsset::NameplateMaterials */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMaterialInterface* GetNameplateMaterial(int32 Index) const;

	/** Returns the Anim Blueprint class to use.
	 * @see UPlayerDataAsset::AnimInstanceClassInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UAnimInstance> GetAnimInstanceClass() const { return AnimInstanceClassInternal; }

	/** Returns the name of a material parameter with a diffuse array.
	 * @see UPlayerDataAsset::SkinSlotNameInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE FName GetSkinArrayParameter() const { return SkinArrayParameterInternal; }

	/** Returns the name of a material parameter with a diffuse index.
	* @see UPlayerDataAsset::SkinSlotNameInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE FName GetSkinIndexParameter() const { return SkinIndexParameterInternal; }

	/** Return first found row by specified player tag. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UPlayerRow* GetRowByPlayerTag(const FPlayerTag& PlayerTag) const;

protected:
	/** All materials that are used by nameplate meshes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Nameplate Materials", ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> NameplateMaterialsInternal;

	/** The AnimBlueprint class to use, can set it only in the gameplay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Anim Instance Class", ShowOnlyInnerProperties))
	TSubclassOf<class UAnimInstance> AnimInstanceClassInternal = nullptr;

	/** The name of a material parameter with a diffuse array. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Skin Array Parameter", ShowOnlyInnerProperties))
	FName SkinArrayParameterInternal = TEXT("DiffuseArray");

	/** The name of a material parameter with a diffuse index. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Skin Index Parameter", ShowOnlyInnerProperties))
	FName SkinIndexParameterInternal = TEXT("DiffuseIndex");
};
