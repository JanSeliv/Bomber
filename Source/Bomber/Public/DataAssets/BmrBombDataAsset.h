// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

#include "BmrBombDataAsset.generated.h"

/**
 * Describes bomb by mesh.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrBombRow final : public UBmrLevelActorRow
{
	GENERATED_BODY()

public:
	/** VFX of the bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	TObjectPtr<class UNiagaraSystem> BombVFX = nullptr;
};

/**
 * Describes common data for all bombs.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrBombDataAsset final : public UBmrLevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrBombDataAsset();

	/** Returns the bomb data asset. */
	static const UBmrBombDataAsset& Get();

	/** Get the bomb lifetime. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE float GetDurationTime() const { return DurationTime; }

	/** Returns the durational gameplay effect applied while the bomb is active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetDurationGameplayEffect() const { return DurationGameplayEffect; }

	/** Returns the explosion damage gameplay effect applied when the bomb detonates. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetExplosionDamageEffect() const { return ExplosionDamageEffect; }

	/** Returns the amount of bomb materials. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetBombMaterialsNum() const { return BombMaterials.Num(); }

	/** Returns the bomb material by specified index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UMaterialInterface* GetBombMaterial(int32 Index) const { return BombMaterials.IsValidIndex(Index) ? BombMaterials[Index] : nullptr; }

	/** Returns associated bomb row by associated instigator actor (e.g: Fori character -> Third (Forest) row).
	 * @param InInstigator - the actor who placed the bomb, used to determine the level type.
	 * @return The bomb row corresponding to the instigator's type, or nullptr if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const UBmrBombRow* GetBombRow(const AActor* InInstigator) const;

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	float DurationTime = 2.f;

	/** Durational gameplay effect applied while the bomb is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<class UGameplayEffect> DurationGameplayEffect = nullptr;

	/** Explosion damage gameplay effect applied when the bomb detonates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<class UGameplayEffect> ExplosionDamageEffect = nullptr;

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> BombMaterials;
};