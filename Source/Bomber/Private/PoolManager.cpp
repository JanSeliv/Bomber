// Copyright (c) Yevhenii Selivanov

#include "PoolManager.h"
//---
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// It's almost farthest possible location where deactivated actors are placed
#define VECTOR_HALF_WORLD_MAX FVector(HALF_WORLD_MAX - HALF_WORLD_MAX * THRESH_VECTOR_NORMALIZED)

// Empty pool object data
const FPoolObject FPoolObject::EmptyObject = FPoolObject();

// Empty pool data container
const FPoolContainer FPoolContainer::EmptyPool = FPoolContainer();

// Parameterized constructor that takes object to keep
FPoolObject::FPoolObject(UObject* InObject)
{
	Object = InObject;
}

// Parameterized constructor that takes a class of the pool
FPoolContainer::FPoolContainer(const UClass* InClass)
{
	ClassInPool = InClass;
}

// Returns the pointer to the Pool element by specified object
FPoolObject* FPoolContainer::FindInPool(const UObject* Object)
{
	if (!Object)
	{
		return nullptr;
	}

	return PoolObjects.FindByPredicate([Object](const FPoolObject& It)
	{
		return It.Object == Object;
	});
}

// Returns the world of an outer
UWorld* UPoolManager::GetWorld() const
{
	if (const AActor* Owner = Cast<AActor>(GetOuter()))
	{
		return Owner->GetWorld();
	}
	return nullptr;
}

// Adds specified object as is to the pool by its class to be handled by the Pool Manager
bool UPoolManager::AddToPool(UObject* Object)
{
	if (!Object)
	{
		return false;
	}

	const UClass* ActorClass = Object->GetClass();
	FPoolContainer* Pool = FindPool(ActorClass);
	if (!Pool)
	{
		const int32 PoolIndex = PoolsInternal.Emplace(FPoolContainer(ActorClass));
		Pool = &PoolsInternal[PoolIndex];
	}

	if (!ensureMsgf(Pool, TEXT("ASSERT: AddToPool: 'Pool' is not valid")))
	{
		return false;
	}

	if (Pool->FindInPool(Object))
	{
		// Already contains in pool
		return false;
	}

	FPoolObject PoolObject(Object);

	if (const AActor* Actor = Cast<AActor>(Object))
	{
		// Decide by its location should it be activated or not
		PoolObject.bIsActive = !Actor->GetActorLocation().Equals(VECTOR_HALF_WORLD_MAX);
	}

	Pool->PoolObjects.Emplace(PoolObject);

	SetActive(PoolObject.bIsActive, Object);

	return true;
}

// Get the object from a pool by specified class
UObject* UPoolManager::TakeFromPool(const FTransform& Transform, const UClass* ClassInPool)
{
	if (!ClassInPool)
	{
		return nullptr;
	}

	FPoolContainer* Pool = FindPool(ClassInPool);
	if (!Pool)
	{
		const int32 PoolIndex = PoolsInternal.Emplace(FPoolContainer(ClassInPool));
		Pool = &PoolsInternal[PoolIndex];
	}

	if (!ensureMsgf(Pool, TEXT("ASSERT: TakeFromPool: 'Pool' is not valid")))
	{
		return nullptr;
	}

	// Try to find ready object to return
	for (FPoolObject& PoolObjectIt : Pool->PoolObjects)
	{
		if (!PoolObjectIt.IsActive())
		{
			if (AActor* Actor = Cast<AActor>(PoolObjectIt.Object))
			{
				Actor->SetActorTransform(Transform);
			}

			SetActive(true, PoolObjectIt.Object);

			return PoolObjectIt.Object;
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Create new object
	UObject* CreatedObject = nullptr;
	if (ClassInPool->IsChildOf<AActor>())
	{
		UClass* ClassToSpawn = const_cast<UClass*>(ClassInPool);
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Cast<AActor>(GetOuter());
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CreatedObject = World->SpawnActor(ClassToSpawn, &Transform, SpawnParameters);
	}
	else
	{
		CreatedObject = NewObject<UObject>(World, ClassInPool);
	}

	if (!ensureMsgf(CreatedObject, TEXT("ASSERT: 'CreatedObject' is not valid")))
	{
		return nullptr;
	}

	Pool->PoolObjects.Emplace(FPoolObject(CreatedObject));

	SetActive(true, CreatedObject);

	return CreatedObject;
}

// Returns the specified object to the pool and deactivates it if the object was taken from the pool before
void UPoolManager::ReturnToPool(UObject* Object)
{
	SetActive(false, Object);
}

// Destroy all object of a pool by a given class
void UPoolManager::EmptyPool(const UClass* ClassInPool)
{
	FPoolContainer* Pool = FindPool(ClassInPool);
	if (!ensureMsgf(Pool, TEXT("ASSERT: 'Pool' is not valid")))
	{
		return;
	}

	TArray<FPoolObject>& PoolObjects = Pool->PoolObjects;
	for (int32 Index = PoolObjects.Num() - 1; Index >= 0; --Index)
	{
		UObject* ObjectIt = PoolObjects.IsValidIndex(Index) ? PoolObjects[Index].Object : nullptr;
		if (!IsValid(ObjectIt))
		{
			continue;
		}

		if (AActor* Actor = Cast<AActor>(ObjectIt))
		{
			Actor->Destroy();
		}
		else
		{
			ObjectIt->ConditionalBeginDestroy();
		}
	}

	PoolObjects.Empty();
}

// Destroy all objects in all pools that are handled by the Pool Manager
void UPoolManager::EmptyAllPools()
{
	const int32 SubPoolsNum = PoolsInternal.Num();
	for (int32 Index = SubPoolsNum - 1; Index >= 0; --Index)
	{
		const UClass* ClassInPool = PoolsInternal.IsValidIndex(Index) ? PoolsInternal[Index].ClassInPool : nullptr;
		EmptyPool(ClassInPool);
	}

	PoolsInternal.Empty();
}

// Activates or deactivates the object if such object is handled by the Pool Manager
void UPoolManager::SetActive(bool bShouldActivate, UObject* Object)
{
	if (!Object)
	{
		return;
	}

	const UClass* ClassInPool = Object ? Object->GetClass() : nullptr;
	FPoolContainer* Pool = FindPool(ClassInPool);
	FPoolObject* PoolObject = Pool ? Pool->FindInPool(Object) : nullptr;
	if (!PoolObject
	    || !PoolObject->IsValid()
	    || PoolObject->IsActive() == bShouldActivate)
	{
		return;
	}

	PoolObject->bIsActive = bShouldActivate;

	if (AActor* Actor = Cast<AActor>(PoolObject->Object))
	{
		if (bShouldActivate)
		{
			Actor->RerunConstructionScripts();
		}
		else
		{
			// SetCollisionEnabled is not replicated, client collides with hidden actor, so move it
			Actor->SetActorLocation(VECTOR_HALF_WORLD_MAX);
		}

		const UWorld* World = GetWorld();
		if (World
		    && World->HasBegunPlay())
		{
			Actor->SetActorHiddenInGame(!bShouldActivate);
			Actor->SetActorEnableCollision(bShouldActivate);
			Actor->SetActorTickEnabled(bShouldActivate);
		}
	}
}

// Returns true if specified object is handled by the Pool Manager and was taken from its pool
bool UPoolManager::IsActive(const UObject* Object) const
{
	const UClass* ClassInPool = Object ? Object->GetClass() : nullptr;
	const FPoolContainer* Pool = FindPool(ClassInPool);;
	const FPoolObject* PoolObject = Pool ? Pool->FindInPool(Object) : nullptr;
	return PoolObject && PoolObject->IsActive();
}

// Returns the pointer to found pool by specified class
FPoolContainer* UPoolManager::FindPool(const UClass* ClassInPool)
{
	if (!ClassInPool)
	{
		return nullptr;
	}
	
	return PoolsInternal.FindByPredicate([ClassInPool](const FPoolContainer& It)
	{
		return It.ClassInPool == ClassInPool;
	});
}
