// Copyright (c) Yevhenii Selivanov

#include "PoolManager.h"
//---
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// It's almost farthest possible location where deactivated actors are placed
#define VECTOR_HALF_WORLD_MAX FVector(HALF_WORLD_MAX - HALF_WORLD_MAX * THRESH_VECTOR_NORMALIZED)

// Returns current world
UWorld* UPoolManager::GetWorld() const
{
	const UEngine* Engine = CastChecked<UEngine>(GetOuter());
	UWorld* FoundWorld = Engine->GetCurrentPlayWorld();

#if WITH_EDITOR
	if (!FoundWorld)
	{
		// If world is not found, most likely a game did not start yet and we are in editor
		const TIndirectArray<FWorldContext>& WorldList = Engine->GetWorldContexts();
		for (int32 i = 0; i < WorldList.Num(); ++i)
		{
			const FWorldContext& WorldContext = WorldList[i];
			if (WorldContext.WorldType == EWorldType::Editor)
			{
				FoundWorld = WorldContext.World();
				break;
			}
		}
	}
#endif // WITH_EDITOR

	ensureMsgf(FoundWorld, TEXT("%s: Can not obtain current world"), *FString(__FUNCTION__));
	return FoundWorld;
}

// Adds specified object as is to the pool by its class to be handled by the Pool Manager
bool UPoolManager::AddToPool(UObject* Object, EPoolObjectState PoolObjectState/* = EPoolObjectState::Inactive*/)
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

	if (!ensureMsgf(Pool, TEXT("%s: 'Pool' is not valid"), *FString(__FUNCTION__)))
	{
		return false;
	}

	if (Pool->FindInPool(Object))
	{
		// Already contains in pool
		return false;
	}

	FPoolObjectData PoolObject(Object);

	if (const AActor* Actor = Cast<AActor>(Object))
	{
		// Decide by its location should it be activated or not if only state is not specified
		switch (PoolObjectState)
		{
		case EPoolObjectState::None:
			PoolObject.bIsActive = !Actor->GetActorLocation().Equals(VECTOR_HALF_WORLD_MAX);
			break;
		case EPoolObjectState::Active:
			PoolObject.bIsActive = true;
			break;
		case EPoolObjectState::Inactive:
			PoolObject.bIsActive = false;
			break;
		default:
			checkf(false, TEXT("%s: Invalid plugin enumeration type. Need to add a handle for that case here"), *FString(__FUNCTION__));
			break;
		}
	}

	Pool->PoolObjects.Emplace(PoolObject);

	SetActive(PoolObject.bIsActive, Object);

	return true;
}

// Get the object from a pool by specified class
UObject* UPoolManager::TakeFromPool(const FTransform& Transform, const UClass* ClassInPool)
{
	if (!ensureMsgf(ClassInPool, TEXT("%s: 'ClassInPool' is not specified"), *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	FPoolContainer* Pool = FindPool(ClassInPool);
	if (!Pool)
	{
		const int32 PoolIndex = PoolsInternal.Emplace(FPoolContainer(ClassInPool));
		Pool = &PoolsInternal[PoolIndex];
	}

	if (!ensureMsgf(Pool, TEXT("%s: 'Pool' is not valid"), *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	// Try to find ready object to return
	for (FPoolObjectData& PoolObjectIt : Pool->PoolObjects)
	{
		if (PoolObjectIt.IsFree())
		{
			UObject* PoolObject = PoolObjectIt.Get();

			if (AActor* Actor = Cast<AActor>(PoolObject))
			{
				Actor->SetActorTransform(Transform);
			}

			SetActive(true, PoolObject);

			return PoolObject;
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
		SpawnParameters.OverrideLevel = World->PersistentLevel; // Always keep new objects on Persistent level
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CreatedObject = World->SpawnActor(ClassToSpawn, &Transform, SpawnParameters);
	}
	else
	{
		CreatedObject = NewObject<UObject>(World, ClassInPool);
	}

	if (!ensureMsgf(CreatedObject, TEXT("%s: 'CreatedObject' is not valid"), *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	Pool->PoolObjects.Emplace(FPoolObjectData(CreatedObject));

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
	if (!ensureMsgf(Pool, TEXT("%s: 'Pool' is not valid"), *FString(__FUNCTION__)))
	{
		return;
	}

	TArray<FPoolObjectData>& PoolObjects = Pool->PoolObjects;
	for (int32 Index = PoolObjects.Num() - 1; Index >= 0; --Index)
	{
		UObject* ObjectIt = PoolObjects.IsValidIndex(Index) ? PoolObjects[Index].Get() : nullptr;
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
	const int32 PoolsNum = PoolsInternal.Num();
	for (int32 Index = PoolsNum - 1; Index >= 0; --Index)
	{
		const UClass* ClassInPool = PoolsInternal.IsValidIndex(Index) ? PoolsInternal[Index].ClassInPool : nullptr;
		EmptyPool(ClassInPool);
	}

	PoolsInternal.Empty();
}

// Destroy all objects in Pool Manager based on a predicate functor
void UPoolManager::EmptyAllByPredicate(TFunctionRef<bool(const UObject* Object)> Predicate)
{
	const int32 PoolsNum = PoolsInternal.Num();
	for (int32 PoolIndex = PoolsNum - 1; PoolIndex >= 0; --PoolIndex)
	{
		if (!PoolsInternal.IsValidIndex(PoolIndex))
		{
			continue;
		}

		TArray<FPoolObjectData>& PoolObjectsRef = PoolsInternal[PoolIndex].PoolObjects;
		const int32 ObjectsNum = PoolObjectsRef.Num();
		for (int32 ObjectIndex = ObjectsNum - 1; ObjectIndex >= 0; --ObjectIndex)
		{
			UObject* ObjectIt = PoolObjectsRef.IsValidIndex(ObjectIndex) ? PoolObjectsRef[ObjectIndex].Get() : nullptr;
			if (!IsValid(ObjectIt)
				|| !Predicate(ObjectIt))
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

			PoolObjectsRef.RemoveAt(ObjectIndex);
		}
	}
}

// Activates or deactivates the object if such object is handled by the Pool Manager
void UPoolManager::SetActive(bool bShouldActivate, UObject* Object)
{
	const UWorld* World = Object ? Object->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const UClass* ClassInPool = Object ? Object->GetClass() : nullptr;
	FPoolContainer* Pool = FindPool(ClassInPool);
	FPoolObjectData* PoolObject = Pool ? Pool->FindInPool(Object) : nullptr;
	if (!PoolObject
		|| !PoolObject->IsValid())
	{
		return;
	}

	PoolObject->bIsActive = bShouldActivate;

	AActor* Actor = PoolObject->Get<AActor>();
	if (!Actor)
	{
		return;
	}

	if (!bShouldActivate)
	{
		// SetCollisionEnabled is not replicated, client collides with hidden actor, so move it
		Actor->SetActorLocation(VECTOR_HALF_WORLD_MAX);
	}

	Actor->SetActorHiddenInGame(!bShouldActivate);
	Actor->SetActorEnableCollision(bShouldActivate);
	Actor->SetActorTickEnabled(bShouldActivate);
}

// Returns current state of specified object
EPoolObjectState UPoolManager::GetPoolObjectState(const UObject* Object) const
{
	const UClass* ClassInPool = Object ? Object->GetClass() : nullptr;
	const FPoolContainer* Pool = FindPool(ClassInPool);;
	const FPoolObjectData* PoolObject = Pool ? Pool->FindInPool(Object) : nullptr;

	if (!PoolObject
		|| !PoolObject->IsValid())
	{
		// Is not contained in any pool
		return EPoolObjectState::None;
	}

	return PoolObject->IsActive() ? EPoolObjectState::Active : EPoolObjectState::Inactive;
}

// Returns true is specified object is handled by Pool Manager
bool UPoolManager::Contains(const UObject* Object) const
{
	return GetPoolObjectState(Object) != EPoolObjectState::None;
}

// Returns true if specified object is handled by the Pool Manager and was taken from its pool
bool UPoolManager::IsActive(const UObject* Object) const
{
	return GetPoolObjectState(Object) == EPoolObjectState::Active;
}

// Returns true if handled object is inactive and ready to be taken from pool
bool UPoolManager::IsFree(const UObject* Object) const
{
	return GetPoolObjectState(Object) == EPoolObjectState::Inactive;
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
