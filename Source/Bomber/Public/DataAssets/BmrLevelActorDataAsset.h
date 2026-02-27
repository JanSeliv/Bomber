// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Bomber.h" // EBmrLevelType, EBmrActorType

// UE
#include "Engine/EngineTypes.h" // ECollisionResponse

#include "BmrLevelActorDataAsset.generated.h"

/**
 * The base archetype of level actor rows. Is implemented in player, powerup rows, etc.
 */
UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew, Const, CollapseCategories, AutoExpandCategories = ("[Bomber]"))
class BOMBER_API UBmrLevelActorRow : public UObject
{
	GENERATED_BODY()

public:
	/** The level where should be used a mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	EBmrLevelType LevelType = ELT::None;

	/** The static mesh, skeletal mesh or texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties, ExposeOnSpawn = "true"))
	TObjectPtr<class UStreamableRenderAsset> Mesh = nullptr;

	/** Returns true if this row contains valid data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	virtual bool IsValid() const { return Mesh != nullptr; }

protected:
#if WITH_EDITOR
	/** Called to handle row changes. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};

/**
 * The base data asset for the Bomber's data.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, Const, AutoExpandCategories = ("[Bomber]"))
class BOMBER_API UBmrBaseDataAsset : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	/** Called to notify on any data asset changes. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};

/**
 * The base data asset for any level actor that contains the main data about them.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class BOMBER_API UBmrLevelActorDataAsset : public UBmrBaseDataAsset
{
	GENERATED_BODY()

public:
	/** Return rows by specified level types in the bitmask. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	void GetRowsByLevelType(
	    TArray<UBmrLevelActorRow*>& OutRows,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrLevelType")) int32 LevelsTypesBitmask) const;

	/** Returns first found row by given predicate function. */
	const UBmrLevelActorRow* GetRowByPredicate(const TFunctionRef<bool(const UBmrLevelActorRow&)>& Predicate) const;

	template <typename T>
	const FORCEINLINE T* GetRowByPredicate(const TFunctionRef<bool(const UBmrLevelActorRow&)>& Predicate) const { return Cast<T>(GetRowByPredicate(Predicate)); }

	/** Return first found row by specified level types. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const UBmrLevelActorRow* GetRowByLevelType(EBmrLevelType LevelType) const;

	template <typename T>
	const FORCEINLINE T* GetRowByLevelType(EBmrLevelType LevelType) const { return Cast<T>(GetRowByLevelType(LevelType)); }

	/** Return first found row by specified mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const UBmrLevelActorRow* GetRowByMesh(const class UStreamableRenderAsset* Mesh) const;

	/** Returns row by index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE UBmrLevelActorRow* GetRowByIndex(int32 Index) const { return Rows.IsValidIndex(Index) ? Rows[Index] : nullptr; }

	template <typename T>
	const FORCEINLINE T* GetRowByIndex(int32 Index) const { return Cast<T>(GetRowByIndex(Index)); }

	/** Returns index of given row, or -1 if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetIndexByRow(const UBmrLevelActorRow* Row) const { return Rows.IndexOfByKey(Row); }

	/** Returns overall number of contained rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetRowsNum() const { return Rows.Num(); }

	/** Returns the class of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	TSubclassOf<class AActor> GetActorClass() const { return ActorClass; }

	/** Returns the actor type of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE EBmrActorType GetActorType() const { return ActorType; }

	/** Asset registry tag key for the actor class path */
	static inline const FName ActorClassTag = TEXT("ActorClass");

	/** Asset registry tag key for the actor type value */
	static inline const FName ActorTypeTag = TEXT("ActorType");

	/** Adds ActorClass and ActorType as asset registry tags for discovery without loading */
	virtual void GetAssetRegistryTags(class FAssetRegistryTagsContext Context) const override;

protected:
	/** DevelopmentOnly: internal class of rows, is overriden by child data assets, used on adding new row. */
	UPROPERTY(BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected, DevelopmentOnly))
	TObjectPtr<UClass> RowClass = UBmrLevelActorRow::StaticClass();

	/** All rows contained by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UBmrLevelActorRow>> Rows;

	/** Class of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<class AActor> ActorClass = nullptr;

	/** Actor type of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	EBmrActorType ActorType = EAT::None;

	/*********************************************************************************************
	 * Collision
	 ********************************************************************************************* */
public:
	/** Returns true if the collision is enabled for an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsEnabledCollision() const { return bEnableCollision; }

	/** Returns an extent size of the collision box of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FVector& GetCollisionExtent() const { return CollisionExtent; }

	/** Returns a response type of the collision box of an actor, whose data is described by this data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE ECollisionResponse GetCollisionResponse() const { return CollisionResponse; }

protected:
	/** If enabled, the Box Collision component is added to actors, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	bool bEnableCollision = true;

	/** Extent size of the collision box of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision", meta = (BlueprintProtected, ShowOnlyInnerProperties, EditCondition = "bEnableCollision"))
	FVector CollisionExtent = FVector(100.f);

	/** Response type of the collision box of an actor, whose data is described by this data asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision", meta = (BlueprintProtected, ShowOnlyInnerProperties, EditCondition = "bEnableCollision"))
	TEnumAsByte<ECollisionResponse> CollisionResponse = ECR_Overlap;

	/*********************************************************************************************
	 * Editor
	 ********************************************************************************************* */
protected:
#if WITH_EDITOR
	/** Handle adding new rows. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};