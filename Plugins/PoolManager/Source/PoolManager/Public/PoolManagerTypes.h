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
	None,
	///< Is not handled by Pool Manager
	Inactive,
	///< Contains in pool, is free and ready to be taken
	Active ///< Was taken from pool and can be returned back.
};

/**
 * Contains the data that describe specific object in a pool.
 */
USTRUCT(BlueprintType)
struct POOLMANAGER_API FPoolObject
{
	GENERATED_BODY()

	/** Empty pool object data. */
	static const FPoolObject EmptyObject;

	/* Default constructor. */
	FPoolObject() = default;

	/* Parameterized constructor that takes object to keep. */
	explicit FPoolObject(UObject* InObject);

	/** Is true whenever the object is taken from the pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	bool bIsActive = false;

	/** The object that is handled by the pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TObjectPtr<UObject> Object = nullptr;

	/** Returns true if the object is taken from the pool. */
	FORCEINLINE bool IsActive() const { return bIsActive && Object; }

	/** Returns true if the object is created. */
	FORCEINLINE bool IsValid() const { return Object != nullptr; }

	/** conversion to "bool" returning true if pool object is valid. */
	FORCEINLINE operator bool() const { return IsValid(); }

	/** Element access. */
	FORCEINLINE UObject* operator->() const { return Object; }
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TObjectPtr<const UClass> ClassInPool = nullptr;

	/** All objects in this pool that are handled by the Pool Manager. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TArray<FPoolObject> PoolObjects;

	/** Returns the pointer to the Pool element by specified object. */
	FPoolObject* FindInPool(const UObject* Object);
	const FORCEINLINE FPoolObject* FindInPool(const UObject* Object) const { return const_cast<FPoolContainer*>(this)->FindInPool(Object); }

	/** Returns true if the class is set for the Pool. */
	FORCEINLINE bool IsValid() const { return ClassInPool != nullptr; }
};