// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataRegistries/BmrLevelActorRow.h"

// Bomber
#include "Structures/BmrPlayerTag.h"

#include "BmrBombRow.generated.h"

/**
 * Row struct for bomb data registered via Data Registry.
 * Same PlayerTag can repeat: gameplay assigns row by player match index % count.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrBombRow : public FBmrLevelActorRow
#if CPP
    , public TBmrLevelActorRow<FBmrBombRow>
#endif
{
	GENERATED_BODY()

	/** The player character this bomb visual belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** VFX of the bomb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UNiagaraSystem> BombVFX = nullptr;

	/** The material instance for the bomb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UMaterialInstance> Material = nullptr;

	/** Finds bomb row data by instigator actor, resolves level type from its MapComponent or SkeletalMeshComponent */
	static const FBmrBombRow& GetBombRow(const class AActor* InInstigator);

	/** Gathers all unique bomb materials from Data Registry */
	static void GetAllBombMaterials(TArray<class UMaterialInterface*>& OutMaterials);

	/** Returns the number of unique bomb materials from Data Registry */
	static int32 GetBombMaterialsNum();

	/** Returns the bomb material by specified index from Data Registry, or nullptr */
	static class UMaterialInterface* GetBombMaterial(int32 Index);
};
