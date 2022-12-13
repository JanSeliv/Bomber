// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UObject/Object.h"
//---
#include "PoolManager.generated.h"

/**
 * States of the object in Pool
 */
UENUM(BlueprintType)
enum class EPoolObjectState : uint8
{
	None, ///< Is not handled by Pool Manager
	Inactive, ///< Contains in pool, is free and ready to be taken
	Active  ///< Was taken from pool and can be returned back.
};

//ENUM_RANGE_BY_FIRST_AND_LAST($ENUM$, $ENUM$::First, $ENUM$::Last);
/**
 * Contains the data that describe specific object in a pool.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FPoolObject
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
	bool bIsActive = false; //[AW]

	/** The object that is handled by the pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TObjectPtr<UObject> Object = nullptr; //[AW]

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
struct BOMBER_API FPoolContainer
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

/** The Pool Manager is used for commonly-spawning objects:
 * Unreal Engine spawning can be very slow. Spawning and destroying things like projectiles,
 * explosions etc might put too much stress on the garbage collector and networking in general,
 * causing different hitches and lags. It's a very good idea for such objects to use the Pool Manager,
 * so instead of creating and destroying anything every time,
 * such objects will be taken from the Pool Manager and can be returned to be deactivated.
 * Deactivated actors are placed outside a level, are hidden, their collisions are disabled and tick is turned off. 
 */
UCLASS(DefaultToInstanced, BlueprintType, Blueprintable)
class BOMBER_API UPoolManager final : public UObject
{
	GENERATED_BODY()

public:
	/** Returns the world of an outer. */
	virtual UWorld* GetWorld() const override;

	/** Adds specified object as is to the pool by its class to be handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DefaultToSelf = "Object"))
	bool AddToPool(UObject* Object = nullptr);

	/** Get the object from a pool by specified class.
	 *  It creates new object if there no free objects contained in pool or does not exist any.
	 *  @return Activated object requested from the pool. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Transform"))
	UObject* TakeFromPool(const FTransform& Transform, const UClass* ClassInPool);

	/** The templated alternative to get the object from a pool by specified class.
	 * @param Transform set the new transform for returned object, if specified class is inherited by AActor
	 * @param ClassInPool is optional parameter, takes the class of given template if null, any child class can be put instead. */
	template <typename T>
	T* TakeFromPool(const FTransform& Transform, const UClass* ClassInPool = nullptr) { return Cast<T>(TakeFromPool(Transform, ClassInPool ? ClassInPool : T::StaticClass())); }

	/** Returns the specified object to the pool and deactivates it if the object was taken from the pool before. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DefaultToSelf = "Object"))
	void ReturnToPool(UObject* Object);

	/** Destroy all object of a pool by a given class. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void EmptyPool(const UClass* ClassInPool);

	/** Destroy all objects in all pools that are handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void EmptyAllPools();

	/** Destroy all objects in Pool Manager based on a predicate functor. */
	void EmptyAllByPredicate(TFunctionRef<bool(const UObject* PoolObject)> Predicate);

	/** Returns current state of specified object. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Object"))
	EPoolObjectState GetPoolObjectState(const UObject* Object) const;

	/** Returns true is specified object is handled by Pool Manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool Contains(const UObject* Object) const;
	
	/** Returns true if specified object is handled by the Pool Manager and was taken from its pool. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Object"))
	bool IsActive(const UObject* Object) const;

	/** Returns true if handled object is inactive and ready to be taken from pool. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Object"))
	bool IsFree(const UObject* Object) const;

protected:
	/** Contains all pools that are handled by the Pool Manger. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Pools", TitleProperty = "ClassInPool"))
	TArray<FPoolContainer> PoolsInternal; //[G]

	/** Returns the pointer to found pool by specified class. */
	FPoolContainer* FindPool(const UClass* ClassInPool);
	const FORCEINLINE FPoolContainer* FindPool(const UClass* ClassInPool) const { return const_cast<UPoolManager*>(this)->FindPool(ClassInPool); }

	/** Activates or deactivates the object if such object is handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DefaultToSelf = "Object"))
	void SetActive(bool bShouldActivate, UObject* Object);
};
