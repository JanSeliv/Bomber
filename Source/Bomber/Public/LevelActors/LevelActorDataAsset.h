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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default")
	ELevelType LevelType = LT::Max  ; //[D]

	/** The static mesh, skeletal mesh or texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++ | Default", meta = (ExposeOnSpawn = "true"))
	class UStreamableRenderAsset* Mesh = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default")
	bool bIsItem = false; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (EditCondition = "bIsItem"))
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Default")
    FORCEINLINE TSubclassOf<class AActor> GetActorClass() const { return ActorClassInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Default")
    FORCEINLINE EActorType GetActorType() const { return ActorTypeInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Default", meta = (AutoCreateRefTerm = "LevelsTypesBitmask"))
	void GetMeshesByLevelType(
		TArray<FLevelActorMeshRow>& OutMeshes,
		UPARAM(meta = (Bitmask, BitmaskEnum = "ELevelType")) const int32& LevelsTypesBitmask) const;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Default")
    FORCEINLINE FVector GetCollisionExtent() const { return CollisionExtentInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Default")
    FORCEINLINE ECollisionResponse GetCollisionResponse() const { return CollisionResponseInternal; }

protected:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (BlueprintProtected, DisplayName = "Actor Class"))
	TSubclassOf<class AActor> ActorClassInternal = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (BlueprintProtected, DisplayName = "Actor Type"))
	EActorType ActorTypeInternal = AT::None; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (BlueprintProtected, DisplayName = "Meshes", ShowOnlyInnerProperties))
	TArray<FLevelActorMeshRow> MeshesInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Collision", meta = (BlueprintProtected, DisplayName = "Collision Extent"))
	FVector CollisionExtentInternal = FVector(100.f); //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Collision", meta = (BlueprintProtected, DisplayName = "Collision Response"))
	TEnumAsByte<ECollisionResponse> CollisionResponseInternal = ECR_Overlap; //[D]
};
