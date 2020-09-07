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

	/** The empty mesh row. */
	static const FLevelActorMeshRow Empty;

	/** Default constructor */
	FLevelActorMeshRow() = default;

	/** Custom constructor to initialize struct with specified level type */
	explicit FLevelActorMeshRow(ELevelType InLevelType) : LevelType(InLevelType) {}

	/** Custom constructor to initialize struct with specified item type */
	explicit FLevelActorMeshRow(EItemType InItemType) : bIsItem(InItemType != EItemType::None), ItemType(InItemType) {}

	/** The level where should be used a mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default")
	ELevelType LevelType = ELT::None; //[D]

	/** The static mesh, skeletal mesh or texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++ | Default", meta = (ExposeOnSpawn = "true"))
	class UStreamableRenderAsset* Mesh = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (InlineEditConditionToggle))
	bool bIsItem = false; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (EditCondition = "bIsItem"))
	EItemType ItemType = EItemType::None; //[D]

	/**
	* Compares types of mesh rows for equality.
	*
	* @param Other The other mesh row being compared.
	* @return true if them types are equal, false otherwise
	*/
	bool IsEqualTypes(const FLevelActorMeshRow& Other) const
	{
		return EnumHasAnyFlags(LevelType, Other.LevelType) && ItemType == Other.ItemType;
	}

	/** Compares mesh rows for equality. */
	FORCEINLINE bool operator==(const FLevelActorMeshRow& Other) const
	{
		return LevelType == Other.LevelType && Mesh == Other.Mesh && ItemType == Other.ItemType;
	}

	/**
	* Creates a hash value from a mesh row.
	*
	* @param MeshRow The mesh to create a hash value for
	* @return The hash value from the components
	*/
	friend FORCEINLINE uint32 GetTypeHash(const FLevelActorMeshRow& MeshRow)
	{
		return GetTypeHash(MeshRow.Mesh);
	}
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class ULevelActorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
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

	/**
	 * Returns the first found mesh row that is equal by its level and item types to specified mesh row types.
	 * @param OutComparedMeshRow Mesh row to compare its level and item types, it true than fill mesh and return it back.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Default", meta = (AutoCreateRefTerm = "OutComparedMeshRow"))
    void GetMeshRowByTypes(FLevelActorMeshRow& OutComparedMeshRow) const;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Collision")
    FORCEINLINE FVector GetCollisionExtent() const { return CollisionExtentInternal; }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Collision")
    FORCEINLINE ECollisionResponse GetCollisionResponse() const { return CollisionResponseInternal; }

protected:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (BlueprintProtected, DisplayName = "Actor Class"))
	TSubclassOf<class AActor> ActorClassInternal = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Default", meta = (BlueprintProtected, DisplayName = "Actor Type"))
	EActorType ActorTypeInternal = EAT::None; //[D]

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
