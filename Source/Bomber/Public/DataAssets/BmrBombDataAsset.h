// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"

#include "BombDataAsset.generated.h"

/**
 * Describes bomb by mesh.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBombRow final : public ULevelActorRow
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
class BOMBER_API UBombDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBombDataAsset();

	/** Returns the bomb data asset. */
	static const UBombDataAsset& Get();

	/** Get the bomb lifetime. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetDuration() const { return DurationInternal; }

	/** Returns the durational gameplay effect applied while the bomb is active.
	 * @see UBombDataAsset::DurationGameplayEffectInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetDurationGameplayEffect() const { return DurationGameplayEffectInternal; }

	/** Returns the explosion damage gameplay effect applied when the bomb detonates.
	 * @see UBombDataAsset::ExplosionDamageEffectInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetExplosionDamageEffect() const { return ExplosionDamageEffectInternal; }

	/** Returns the amount of bomb materials. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetBombMaterialsNum() const { return BombMaterialsInternal.Num(); }

	/** Returns the bomb material by specified index.
	 * @see UBombDataAsset::BombMaterialInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UMaterialInterface* GetBombMaterial(int32 Index) const { return BombMaterialsInternal.IsValidIndex(Index) ? BombMaterialsInternal[Index] : nullptr; }

	/** Returns associated bomb row by associated instigator actor (e.g: Fori character -> Third (Forest) row).
	 * @param InInstigator - the actor who placed the bomb, used to determine the level type.
	 * @return The bomb row corresponding to the instigator's type, or nullptr if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const UBombRow* GetBombRow(const AActor* InInstigator) const;

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Duration", ShowOnlyInnerProperties))
	float DurationInternal = 2.f;

	/** Durational gameplay effect applied while the bomb is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Duration Gameplay Effect", ShowOnlyInnerProperties))
	TSubclassOf<class UGameplayEffect> DurationGameplayEffectInternal = nullptr;

	/** Explosion damage gameplay effect applied when the bomb detonates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Explosion Damage Effect", ShowOnlyInnerProperties))
	TSubclassOf<class UGameplayEffect> ExplosionDamageEffectInternal = nullptr;

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Bomb Materials", ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> BombMaterialsInternal;
};