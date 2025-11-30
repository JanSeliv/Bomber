// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

// Bomber
#include "Structures/BmrPlayerTag.h"

#include "BmrPlayerDataAsset.generated.h"

/**
 * Determines each mesh to attach.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrAttachedMesh
{
	GENERATED_BODY()

	/** The attached static mesh or skeletal mesh.  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties, ExposeOnSpawn))
	TObjectPtr<UStreamableRenderAsset> AttachedMesh = nullptr;

	/** In the which socket should attach this prop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FName Socket = NAME_None;

	/** Prop animation is loop played all the time, starts playing on attaching to the owner. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimSequence> MeshAnimation = nullptr;
};

/**
 * The player archetype of level actor rows. Determines the individual of the character model
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPlayerRow final : public UBmrLevelActorRow
{
	GENERATED_BODY()

public:
	/** The tag of this player character to be used for association of this player with other data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Row", meta = (ShowOnlyInnerProperties))
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** Gameplay effect to apply on changing the character from one to another. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	TSubclassOf<class UGameplayEffect> ConfigGameplayEffect = nullptr;

	/** All meshes that will be attached to the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TArray<FBmrAttachedMesh> PlayerProps;

	/** The own movement animation for the each character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UBlendSpace1D> IdleWalkRunBlendSpace = nullptr;

	/** Dance animation that is used mostly in the menu instead of idle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimSequence> DanceAnimation = nullptr;

	/** Death animation montage that is played on the character death. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimMontage> DeathMontage = nullptr;

	/** Returns the num of skin textures in the array of diffuse maps specified a player material instance.
	 * @return The num of skin textures or INDEX_NONE if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetSkinTexturesNum() const;

	/** Returns the dynamic material instance of a player with specified skin.
	 * @param SkinIndex The skin position to get.
	 * @see UBmrPlayerRow::MaterialInstancesDynamic */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UMaterialInstanceDynamic* GetMaterialInstanceDynamic(int32 SkinIndex) const;

	/** Creates dynamic material instance for each skin if is not done before.
	 * @see UBmrPlayerRow::MaterialInstancesDynamic */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void UpdateSkinTextures();

protected:
	/** The material instance of a player.
	 * @warning Is not BlueprintReadOnly and has not getter to prevent being used directly, we have dynamic materials instead.
	 * @see UBmrPlayerRow::MaterialInstancesDynamic. */
	UPROPERTY(EditDefaultsOnly, Category = "Row", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<class UMaterialInstance> MaterialInstance = nullptr;

	/**
	 * Contains all created dynamic materials for each skin in the Material Instance.
	 * Saves memory avoiding creation of dynamic materials for each mesh component, just use the same dynamic material for different meshes with the same skin.
	 * Is filled on object creating and changing.
	 * @warning Is NOT transient as it is set in skeletal meshes; if do transient, it will fail the cook with the import error.
	 * @see UBmrPlayerRow::MaterialInstance. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "Row", meta = (BlueprintProtected))
	TArray<TObjectPtr<class UMaterialInstanceDynamic>> MaterialInstancesDynamic;

#if WITH_EDITOR
	/** Handle adding and changing material instance to prepare dynamic materials. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};

/**
 * The data asset of the Bomber characters
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPlayerDataAsset final : public UBmrLevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrPlayerDataAsset();

	/** Returns the player data asset. */
	static const UBmrPlayerDataAsset& Get();

	/** The num of nameplate materials.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetNameplateMaterialsNum() const { return NameplateMaterials.Num(); }

	/** Returns a nameplate material by index, is used by nameplate meshes.
	 * @see UBmrPlayerDataAsset::NameplateMaterials */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UMaterialInterface* GetNameplateMaterial(int32 Index) const;

	/** Returns the Anim Blueprint class to use. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<class UAnimInstance> GetAnimInstanceClass() const { return AnimInstanceClass; }

	/** Returns the name of a material parameter with a diffuse array. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE FName GetSkinArrayParameter() const { return SkinArrayParameter; }

	/** Returns the name of a material parameter with a diffuse index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE FName GetSkinIndexParameter() const { return SkinIndexParameter; }

	/** Return first found row by specified player tag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "PlayerTag"))
	const UBmrPlayerRow* GetRowByPlayerTag(const FBmrPlayerTag& PlayerTag) const;

	/** Returns the number of startup abilities that will be granted to the player at the start of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetStartupAbilitiesNum() const { return StartupAbilities.Num(); }

	/** Returns the startup ability by index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<class UGameplayAbility> GetStartupAbility(int32 Index) const { return StartupAbilities.IsValidIndex(Index) ? StartupAbilities[Index] : nullptr; }

	/** Returns the gameplay effect that gives immune to incoming damage when applied. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetBlockIncomingDamageEffect() const { return BlockIncomingDamageEffect; }

	/** Returns the gameplay effect that disables movement for the player character when applied. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetBlockMovementEffect() const { return BlockMovementEffect; }

protected:
	/** All materials that are used by nameplate meshes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> NameplateMaterials;

	/** The AnimBlueprint class to use, can set it only in the gameplay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<class UAnimInstance> AnimInstanceClass = nullptr;

	/** The name of a material parameter with a diffuse array. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	FName SkinArrayParameter = TEXT("DiffuseArray");

	/** The name of a material parameter with a diffuse index. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	FName SkinIndexParameter = TEXT("DiffuseIndex");

	/** Contains all abilities to grant on the player at the start of the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<TSubclassOf<class UGameplayAbility>> StartupAbilities;

	/** When applied, gives immune to incoming damage.
	 * Adds BmrGameplayTags::GameplayEffect::Block::IncomingDamage tag.
	 * E.g: Is used by God cheat, might be useful for shield skill, or when player joins existing game in progress. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<class UGameplayEffect> BlockIncomingDamageEffect = nullptr;

	/** When applied, disables movement for the player character.
	 * Adds BmrGameplayTags::GameplayEffect::Block::Movement tag.
	 * E.g: Is used when player is in a menu, during 3-2-1 timer, when died, in cinematics etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<class UGameplayEffect> BlockMovementEffect = nullptr;
};