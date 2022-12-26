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
 * Can be used in Editor before game starts.
 */
UCLASS(BlueprintType, Blueprintable)
class POOLMANAGER_API UPoolManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#pragma region GetPoolManager
	/** Returns the Pool Manager, is checked and wil crash if can't be obtained.
	* UPoolManager::Get(). with no parameters can be used in most cases if there is no specific set up.
	* @tparam T is optional, put your child class if you implemented your own Pull Manager. */
	template <typename T = ThisClass>
	static FORCEINLINE T& Get() { return *CastChecked<T>(GetPoolManager(T::StaticClass())); }

	/** Returns the pointer to the Pool Manager.
	 * @param OptionalClass is optional, specify the class if you implemented your own Pool Manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE UPoolManager* GetPoolManager(TSubclassOf<UPoolManager> OptionalClass = nullptr) { return GEngine ? Cast<UPoolManager>(GEngine->GetEngineSubsystemBase(OptionalClass ? *OptionalClass : StaticClass())) : nullptr; }
#pragma endregion GetPoolManager

	/** Returns current world. */
	virtual UWorld* GetWorld() const override;

	/** Adds specified object as is to the pool by its class to be handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DefaultToSelf = "Object"))
	virtual bool AddToPool(UObject* Object = nullptr, EPoolObjectState PoolObjectState = EPoolObjectState::Inactive);

	/** Get the object from a pool by specified class.
	 *  It creates new object if there no free objects contained in pool or does not exist any.
	 *  @return Activated object requested from the pool. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Transform"))
	virtual UObject* TakeFromPool(const FTransform& Transform, const UClass* ClassInPool);

	/** The templated alternative to get the object from a pool by specified class.
	 * @param Transform set the new transform for returned object, if specified class is inherited by AActor
	 * @param ClassInPool is optional parameter, takes the class of given template if null, any child class can be put instead. */
	template <typename T>
	FORCEINLINE T* TakeFromPool(const FTransform& Transform, const UClass* ClassInPool = nullptr) { return Cast<T>(TakeFromPool(Transform, ClassInPool ? ClassInPool : T::StaticClass())); }

	/** Returns the specified object to the pool and deactivates it if the object was taken from the pool before. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DefaultToSelf = "Object"))
	virtual void ReturnToPool(UObject* Object);

	/** Destroy all object of a pool by a given class. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	virtual void EmptyPool(const UClass* ClassInPool);

	/** Destroy all objects in all pools that are handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	virtual void EmptyAllPools();

	/** Destroy all objects in Pool Manager based on a predicate functor. */
	virtual void EmptyAllByPredicate(TFunctionRef<bool(const UObject* PoolObject)> Predicate);

	/** Returns current state of specified object. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Object"))
	virtual EPoolObjectState GetPoolObjectState(const UObject* Object) const;

	/** Returns true is specified object is handled by Pool Manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	virtual bool Contains(const UObject* Object) const;

	/** Returns true if specified object is handled by the Pool Manager and was taken from its pool. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Object"))
	virtual bool IsActive(const UObject* Object) const;

	/** Returns true if handled object is inactive and ready to be taken from pool. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DefaultToSelf = "Object"))
	virtual bool IsFree(const UObject* Object) const;

protected:
	/** Contains all pools that are handled by the Pool Manger. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Pools"))
	TArray<FPoolContainer> PoolsInternal;

	/** Is called on initialization of the Pool Manager instance. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Returns the pointer to found pool by specified class. */
	virtual FPoolContainer* FindPool(const UClass* ClassInPool);
	const FORCEINLINE FPoolContainer* FindPool(const UClass* ClassInPool) const { return const_cast<UPoolManager*>(this)->FindPool(ClassInPool); }

	/** Activates or deactivates the object if such object is handled by the Pool Manager. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DefaultToSelf = "Object"))
	virtual void SetActive(bool bShouldActivate, UObject* Object);
};
