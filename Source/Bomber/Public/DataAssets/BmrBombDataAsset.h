// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

#include "BmrBombDataAsset.generated.h"

/**
 * Contains configuration data for bombs.
 * Content is stored in FBmrBombRow Data Registry rows
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

	/** Finds bomb row data by instigator actor, resolves level type from its MapComponent or SkeletalMeshComponent */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const struct FBmrBombRow& GetBombRow(const AActor* InInstigator);

	/** Returns the amount of unique bomb materials from Data Registry */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static int32 GetBombMaterialsNum();

	/** Returns the bomb material by specified index from Data Registry, or nullptr */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static class UMaterialInterface* GetBombMaterial(int32 Index);

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
};