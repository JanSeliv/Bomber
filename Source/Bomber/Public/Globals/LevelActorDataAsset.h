// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
//---
#include "Engine/DataAsset.h"
//---
#include "LevelActorDataAsset.generated.h"

/**
 * The base archetype of level actor rows. Is implemented in player, item rows, etc.
 */
UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew, Const, AutoExpandCategories=("C++"))
class ULevelActorRow : public UObject
{
	GENERATED_BODY()

public:
	/** The level where should be used a mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	ELevelType LevelType = ELT::None; //[D]

	/** The static mesh, skeletal mesh or texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties, ExposeOnSpawn = "true"))
	TObjectPtr<class UStreamableRenderAsset> Mesh = nullptr; //[D]

protected:
#if WITH_EDITOR
	/** Called to handle row changes. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
};

/**
 * The base data asset for the Bomber's data.
 */
UCLASS(Blueprintable, BlueprintType, Abstract, Const, AutoExpandCategories=("C++"))
class UBomberDataAsset : public UDataAsset
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	/** Called to notify on any data asset changes. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
};

/**
 * The base data asset for any level actor that contains the main data about them.
 */
UCLASS(Blueprintable, BlueprintType)
class ULevelActorDataAsset : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Return rows by specified level types in the bitmask. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetRowsByLevelType(
		TArray<ULevelActorRow*>& OutRows,
		UPARAM(meta = (Bitmask, BitmaskEnum = "ELevelType")) int32 LevelsTypesBitmask) const;

	/** Return first found row by specified level types. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class ULevelActorRow* GetRowByLevelType(ELevelType LevelType) const;

	/** Return first found row by specified level types. */
	template <typename T>
	const FORCEINLINE T* GetRowByLevelType(ELevelType LevelType) const { return Cast<T>(GetRowByLevelType(LevelType)); }

	/** Return first found row by specified mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class ULevelActorRow* GetRowByMesh(const class UStreamableRenderAsset* Mesh) const;

	/** Return first found row by specified mesh. */
	template <typename T>
	const FORCEINLINE T* GetRowByMesh(const class UStreamableRenderAsset* Mesh) const { return Cast<T>(GetRowByMesh(Mesh)); }

	/** Returns overall number of contained rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetRowsNum() const { return RowsInternal.Num(); }

	/** Returns the class of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE UClass* GetActorClass() const { return ActorClassInternal; }

	/** Returns the actor type of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE EActorType GetActorType() const { return ActorTypeInternal; }

	/** Returns a extent size of the collision box of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FVector& GetCollisionExtent() const { return CollisionExtentInternal; }

	/** Returns a response type of the collision box of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE ECollisionResponse GetCollisionResponse() const { return CollisionResponseInternal; }

protected:
	/** DevelopmentOnly: internal class of rows, is overriden by child data assets, used on adding new row. */
	UPROPERTY(BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Row Class", DevelopmentOnly))
	TObjectPtr<UClass> RowClassInternal = ULevelActorRow::StaticClass();

	/** All rows contained by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, meta = (BlueprintProtected, DisplayName = "Rows", ShowOnlyInnerProperties))
	TArray<TObjectPtr<class ULevelActorRow>> RowsInternal; //[D]

	/** Class of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Actor Class", ShowOnlyInnerProperties))
	TSubclassOf<class AActor> ActorClassInternal; //[D]

	/** Actor type of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Actor Type", ShowOnlyInnerProperties))
	EActorType ActorTypeInternal = EAT::None; //[D]

	/** Extent size of the collision box of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collision Extent", ShowOnlyInnerProperties))
	FVector CollisionExtentInternal = FVector(100.f); //[D]

	/** Response type of the collision box of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collision Response", ShowOnlyInnerProperties))
	TEnumAsByte<ECollisionResponse> CollisionResponseInternal = ECR_Overlap; //[D]

#if WITH_EDITOR
	/** Handle adding new rows. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif	//WITH_EDITOR
};
