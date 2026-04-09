// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

// Bomber
#include "Structures/BmrPlayerTag.h"

#include "BmrPlayerDataAsset.generated.h"

/**
 * Contains configuration data for Bomber characters.
 * Content is stored in FBmrPlayerRow, FBmrPlayerPropRow and FBmrPlayerSkinRow Data Registry rows
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

	/** Returns the Data Registry row name for the given player tag, or NAME_None */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FName GetRowNameByPlayerTag(const FBmrPlayerTag& PlayerTag);

	/** Returns the total number of skin materials for the given player tag from FBmrPlayerSkinRow */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static int32 GetSkinTexturesNum(const FBmrPlayerTag& PlayerTag);

	/** Returns the skin material for the given player tag and skin index from FBmrPlayerSkinRow, or nullptr */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static class UMaterialInterface* GetSkinMaterial(const FBmrPlayerTag& PlayerTag, int32 SkinIndex);

	/** Returns player row data by mesh from Data Registry, returns empty row data if not found */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const struct FBmrPlayerRow& GetRowByMesh(const class UStreamableRenderAsset* Mesh);

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