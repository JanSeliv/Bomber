// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"
//---
#include "PoolManagerTypes.h"
//---
#include "PoolManager.generated.h"

/** The Pool Manager is used for commonly-spawning objects:
 * Unreal Engine spawning can be very slow. Spawning and destroying things like projectiles,
 * explosions etc might put too much stress on the garbage collector and networking in general,
 * causing different hitches and lags. It's a very good idea for such objects to use the Pool Manager,
 * so instead of creating and destroying anything every time,
 * such objects will be taken from the Pool Manager and can be returned to be deactivated.
 * Deactivated actors are placed outside a level, are hidden, their collisions are disabled and tick is turned off. 
 */
UCLASS(DefaultToInstanced, BlueprintType, Blueprintable)
class POOLMANAGER_API UPoolManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns the world of an outer. */
	virtual UWorld* GetWorld() const override;

	template <typename T = ThisClass>
	static FORCEINLINE T& Get() { return *CastChecked<UPoolManager>(GetPoolManager()); }

	/** Returns this Pool Manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE UPoolManager* GetPoolManager() { return GEngine ? GEngine->GetEngineSubsystem<ThisClass>() : nullptr; }

	/** Adds specified object as is to the pool by its class to be handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DefaultToSelf = "Object"))
	bool AddToPool(UObject* Object = nullptr, EPoolObjectState PoolObjectState = EPoolObjectState::Inactive);

	/** Get the object from a pool by specified class.
	 *  It creates new object if there no free objects contained in pool or does not exist any.
	 *  @return Activated object requested from the pool. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Transform"))
	UObject* TakeFromPool(const FTransform& Transform, const UClass* ClassInPool);

	/** The templated alternative to get the object from a pool by specified class.
	 * @param Transform set the new transform for returned object, if specified class is inherited by AActor
	 * @param ClassInPool is optional parameter, takes the class of given template if null, any child class can be put instead. */
	template <typename T>
	FORCEINLINE T* TakeFromPool(const FTransform& Transform, const UClass* ClassInPool = nullptr) { return Cast<T>(TakeFromPool(Transform, ClassInPool ? ClassInPool : T::StaticClass())); }

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
	TArray<FPoolContainer> PoolsInternal;

	/** Returns the pointer to found pool by specified class. */
	FPoolContainer* FindPool(const UClass* ClassInPool);
	const FORCEINLINE FPoolContainer* FindPool(const UClass* ClassInPool) const { return const_cast<UPoolManager*>(this)->FindPool(ClassInPool); }

	/** Activates or deactivates the object if such object is handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DefaultToSelf = "Object"))
	void SetActive(bool bShouldActivate, UObject* Object);
};
