// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UObject/Object.h"
//---
#include "PoolManagerTypes.generated.h"

/**
 * States of the object in Pool
 */
UENUM(BlueprintType)
enum class EPoolObjectState : uint8
{
	///< Is not handled by Pool Manager
	None,
	///< Contains in pool, is free and ready to be taken
	Inactive,
	///< Was taken from pool and can be returned back.
	Active
};

/**
 * Contains the data that describe specific object in a pool.
 */
USTRUCT(BlueprintType)
struct POOLMANAGER_API FPoolObjectData
{
	GENERATED_BODY()

	/** Empty pool object data. */
	static const FPoolObjectData EmptyObject;

	/* Default constructor. */
	FPoolObjectData() = default;

	/* Parameterized constructor that takes object to keep. */
	explicit FPoolObjectData(UObject* InPoolObject);

	/** Is true whenever the object is taken from the pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	/** The object that is handled by the pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UObject> PoolObject = nullptr;

	/** Returns true if the object is taken from the pool. */
	FORCEINLINE bool IsActive() const { return bIsActive && IsValid(); }

	/** Returns true if handled object is inactive and ready to be taken from pool. */
	FORCEINLINE bool IsFree() const { return !bIsActive && IsValid(); }

	/** Returns true if the object is created. */
	FORCEINLINE bool IsValid() const { return PoolObject != nullptr; }

	/** conversion to "bool" returning true if pool object is valid. */
	FORCEINLINE operator bool() const { return IsValid(); }

	/** Element access. */
	template <typename T = UObject>
	FORCEINLINE T* Get() const { return Cast<T>(PoolObject.Get()); }

	/** Element access. */
	FORCEINLINE UObject* operator->() const { return PoolObject.Get(); }
};

/**
 * Keeps the objects by class to be handled by the Pool Manager.
 */
USTRUCT(BlueprintType)
struct POOLMANAGER_API FPoolContainer
{
	GENERATED_BODY()

	/** Empty pool data container. */
	static const FPoolContainer EmptyPool;

	/* Default constructor. */
	FPoolContainer() = default;

	/* Parameterized constructor that takes a class of the pool. */
	explicit FPoolContainer(const UClass* InClass);

	/** Class of all objects in this pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const UClass> ClassInPool = nullptr;

	/** All objects in this pool that are handled by the Pool Manager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPoolObjectData> PoolObjects;

	/** Returns the pointer to the Pool element by specified object. */
	FPoolObjectData* FindInPool(const UObject* Object);
	const FORCEINLINE FPoolObjectData* FindInPool(const UObject* Object) const { return const_cast<FPoolContainer*>(this)->FindInPool(Object); }

	/** Returns true if the class is set for the Pool. */
	FORCEINLINE bool IsValid() const { return ClassInPool != nullptr; }
};
