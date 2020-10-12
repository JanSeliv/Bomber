// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
//---
#include "Engine/DataAsset.h"
//---
#include "LevelActorDataAsset.generated.h"

/**
 * The base archetype of level actor rows. Is implemented in player, item rows, etc.
 */
UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew)
class ULevelActorRow : public UObject
{
	GENERATED_BODY()

public:
	/** The level where should be used a mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	ELevelType LevelType = ELT::None; //[D]

	/** The static mesh, skeletal mesh or texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Row", meta = (ExposeOnSpawn = "true"))
	class UStreamableRenderAsset* Mesh = nullptr; //[D]
};

/**
 * The base data asset for any level actor that contains the main data about them.
 */
UCLASS(Blueprintable, BlueprintType)
class ULevelActorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetRowsByLevelType(
		TArray<class ULevelActorRow*>& OutRows,
		UPARAM(meta = (Bitmask, BitmaskEnum = "ELevelType")) int32 LevelsTypesBitmask) const;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetRowsNum() const { return RowsInternal.Num(); }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE TSubclassOf<class AActor> GetActorClass() const { return ActorClassInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE EActorType GetActorType() const { return ActorTypeInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE FVector GetCollisionExtent() const { return CollisionExtentInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE ECollisionResponse GetCollisionResponse() const { return CollisionResponseInternal; }

protected:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, meta = (BlueprintProtected, DisplayName = "Rows", ShowOnlyInnerProperties))
	TArray<class ULevelActorRow*> RowsInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Actor Class", ShowOnlyInnerProperties))
	TSubclassOf<class AActor> ActorClassInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Actor Type", ShowOnlyInnerProperties))
	EActorType ActorTypeInternal = EAT::None; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collision Extent", ShowOnlyInnerProperties))
	FVector CollisionExtentInternal = FVector(100.f); //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collision Response", ShowOnlyInnerProperties))
	TEnumAsByte<ECollisionResponse> CollisionResponseInternal = ECR_Overlap; //[D]
};
