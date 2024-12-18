// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "FTGTypes.h"
//---
#include "FTGComponent.generated.h"

class UFTGDataAsset;
class UStaticMesh;

/**
 * Is main logic component that generates foot trails.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FOOTTRAILSGENERATORRUNTIME_API UFTGComponent : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public function
	 ********************************************************************************************* */
public:
	/** Sets default values for this component's properties. */
	UFTGComponent();

	/** Returns the data asset that contains all the assets and tweaks of Foot Trails game feature.
	 * @see UFootTrailsGeneratorComponent::FootTrailsDataAssetInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class UFTGDataAsset* GetFootTrailsDataAsset() const;

	/** Guarantees that the data asset is loaded, otherwise, it will crash. */
	const class UFTGDataAsset& GetFootTrailsDataAssetChecked() const;

	/** Returns the random foot trail instance for given types. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UStaticMesh* GetRandomMesh(EFTGTrailType FootTrailType) const;

	/** Returns the instanced static mesh actor that contains all foot trails, if spawned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class AInstancedStaticMeshActor* GetInstancedStaticMeshActor() const { return InstancedStaticMeshActorInternal; }

	/** Loads all foot trails archetypes, should be called only once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitOnce();

	/** Triggers the generation of foot trails.
	 * @warning is implemented in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "C++", DisplayName = "Generate Foot Trails")
	void BPGenerateFootTrails();

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains all the assets and tweaks of Foot Trails game feature. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Foot Trails Data Asset"))
	TSoftObjectPtr<const class UFTGDataAsset> FootTrailsDataAssetInternal = nullptr;

	/** Converts actors with static meshes to instanced static meshes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Instanced Static Mesh Actor"))
	TObjectPtr<class AInstancedStaticMeshActor> InstancedStaticMeshActorInternal = nullptr;

	/** Loaded foot trails archetypes for all levels. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Foot Trail Instances"))
	TMap<FFTGArchetype, TObjectPtr<UStaticMesh>> FootTrailInstancesInternal;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Called when the game ends. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Spawns given Foot Trail by its type on the specified cell. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SpawnFootTrail(EFTGTrailType FootTrailType, const struct FCell& Cell, float CellRotation);
};
