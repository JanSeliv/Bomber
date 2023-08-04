// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "FootTrailsTypes.h"
//---
#include "FootTrailsGeneratorComponent.generated.h"

class UFootTrailsDataAsset;
class UStaticMesh;

/**
 * Is main logic component that generates foot trails.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FOOTTRAILSGENERATORRUNTIME_API UFootTrailsGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public function
	 ********************************************************************************************* */
public:
	/** Sets default values for this component's properties. */
	UFootTrailsGeneratorComponent();

	/** Returns the data asset that contains all the assets and tweaks of Foot Trails game feature.
	 * @see UFootTrailsGeneratorComponent::FootTrailsDataAssetInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE class UFootTrailsDataAsset* GetFootTrailsDataAsset() const { return FootTrailsDataAssetInternal.LoadSynchronous(); }

	/** Guarantees that the data asset is loaded, otherwise, it will crash. */
	const class UFootTrailsDataAsset& GetFootTrailsDataAssetChecked() const;

	/** Returns the random foot trail instance for given types. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UStaticMesh* GetRandomMesh(EFootTrailType FootTrailType) const;

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains all the assets and tweaks of Foot Trails game feature. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Foot Trails Data Asset"))
	TSoftObjectPtr<const class UFootTrailsDataAsset> FootTrailsDataAssetInternal = nullptr;

	/** Converts actors with static meshes to instanced static meshes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Instanced Static Mesh Actor"))
	TObjectPtr<class AInstancedStaticMeshActor> InstancedStaticMeshActorInternal = nullptr;

	/** Loaded foot trails archetypes for all levels. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Foot Trail Instances"))
	TMap<FFootTrailArchetype, TObjectPtr<UStaticMesh>> FootTrailInstancesInternal;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Called when the game ends. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Loads all foot trails archetypes. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void Init();

	/** Spawns given Foot Trail by its type on the specified cell. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SpawnFootTrail(EFootTrailType FootTrailType, const struct FCell& Cell, float CellRotation);
};
