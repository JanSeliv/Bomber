// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"
//---
#include "BombDataAsset.generated.h"

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
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetLifeSpan() const { return LifeSpanInternal; }

	/** Returns the amount of bomb materials. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetBombMaterialsNum() const { return BombMaterialsInternal.Num(); }

	/** Returns the bomb material by specified index.
	 * @see UBombDataAsset::BombMaterialInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMaterialInterface* GetBombMaterial(int32 Index) const { return BombMaterialsInternal.IsValidIndex(Index) ? BombMaterialsInternal[Index] : nullptr; }

	/** Get the bomb explosion VFX. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNiagaraSystem* GetExplosionVFX() const { return ExplosionVFXInternal; }

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Life Span", ShowOnlyInnerProperties))
	float LifeSpanInternal = 2.f;

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Bomb Materials", ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> BombMaterialsInternal;

	/** The emitter of the bomb explosion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Explosion Particle", ShowOnlyInnerProperties))
	TObjectPtr<class UNiagaraSystem> ExplosionVFXInternal = nullptr;
};