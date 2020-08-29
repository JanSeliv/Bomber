// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
//---
#include "Engine/DataAsset.h"
//---
#include "LevelActorDataAsset.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FLevelActorMeshRow
{
	GENERATED_BODY()

	/** Default constructor */
	FLevelActorMeshRow() = default;

	/** Custom constructor to initialize struct with specified level type */
	explicit FLevelActorMeshRow(ELevelType InLevelType) : LevelType(InLevelType) {}

	/** The level where should be used a mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	ELevelType LevelType = ELevelType::LT_Max; //[D]

	/** The static mesh, skeletal mesh or texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (ExposeOnSpawn = "true"))
	class UStreamableRenderAsset* Mesh = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	bool bIsItem = false; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (EditCondition = "bIsItem"))
	EItemType ItemType = EItemType::None; //[D]
};

/**
 *
 */
UCLASS(Abstract)
class ULevelActorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor */
	ULevelActorDataAsset();

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE TSubclassOf<class AActor> GetActorClass() const { return ActorClassInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE EActorType GetActorType() const { return ActorTypeInternal; }

protected:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected))
	TSubclassOf<class AActor> ActorClassInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected))
	EActorType ActorTypeInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<FLevelActorMeshRow> MeshesInternal; //[D]
};
