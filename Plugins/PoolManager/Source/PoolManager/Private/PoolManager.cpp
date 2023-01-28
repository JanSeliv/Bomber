// Copyright (c) Yevhenii Selivanov

#include "PoolManager.h"
//---
#include "Engine/World.h"
#include "GameFramework/Actor.h"
//---
#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

// It's almost farthest possible location where deactivated actors are placed
#define VECTOR_HALF_WORLD_MAX FVector(HALF_WORLD_MAX - HALF_WORLD_MAX * THRESH_VECTOR_NORMALIZED)

// Returns the pointer to your Pool Manager
UPoolManager* UPoolManager::GetPoolManager(TSubclassOf<UPoolManager> OptionalClass/* = nullptr*/, const UObject* OptionalWorldContext/* = nullptr*/)
{
	if (!OptionalClass)
	{
		OptionalClass = StaticClass();
	}

	const UWorld* FoundWorld = OptionalWorldContext
		                           ? GEngine->GetWorldFromContextObject(OptionalWorldContext, EGetWorldErrorMode::Assert)
		                           : GEngine->GetCurrentPlayWorld();

#if WITH_EDITOR
	if (!FoundWorld && GEditor)
	{
		// If world is not found, most likely a game did not start yet and we are in editor
		const FWorldContext& WorldContext = GEditor->IsPlaySessionInProgress() ? *GEditor->GetPIEWorldContext() : GEditor->GetEditorWorldContext();
		FoundWorld = WorldContext.World();
	}
#endif // WITH_EDITOR

	if (!ensureMsgf(FoundWorld, TEXT("%s: Can not obtain current world"), *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	UPoolManager* FoundPoolManager = Cast<UPoolManager>(FoundWorld->GetSubsystemBase(OptionalClass));
	if (!ensureMsgf(FoundPoolManager, TEXT("%s: 'Can not find Pool Manager for %s class in %s world"), *FString(__FUNCTION__), *OptionalClass->GetName(), *FoundWorld->GetName()))
	{
		return nullptr;
	}

	return FoundPoolManager;
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
	UObject* CreatedObject;
	if (ClassInPool->IsChildOf<AActor>())
	{
		UClass* ClassToSpawn = const_cast<UClass*>(ClassInPool);
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.OverrideLevel = World->PersistentLevel; // Always keep new objects on Persistent level
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.bDeferConstruction = true; // Delay construction to add it to the pool first
		CreatedObject = World->SpawnActor(ClassToSpawn, &Transform, SpawnParameters);
	}
	else
	{
		CreatedObject = NewObject<UObject>(World, ClassInPool);
	}

	checkf(CreatedObject, TEXT("CRITICAL ERROR: %s: 'CreatedObject' is not valid"), *FString(__FUNCTION__))

	FPoolObjectData PoolObjectData;
	PoolObjectData.PoolObject = CreatedObject;
	// Set activity here instead of calling UPoolManager::SetActive since new object never was inactivated before to switch the state
	// Is true because it returns new active object to the game
	PoolObjectData.bIsActive = true;
	Pool->PoolObjects.Emplace(MoveTemp(PoolObjectData));

	if (AActor* SpawnedActor = Cast<AActor>(CreatedObject))
	{
		// Call construction script since it was delayed before to add it to the pool first
		SpawnedActor->FinishSpawning(Transform);
	}

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
	const FPoolContainer* Pool = FindPool(ClassInPool);
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

// Returns true if object is known by Pool Manager
bool UPoolManager::IsRegistered(const UObject* Object) const
{
	return GetPoolObjectState(Object) != EPoolObjectState::None;
}

// Is called on initialization of the Pool Manager instance
void UPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	if (GEditor
		&& !GEditor->IsPlaySessionInProgress() // Is Editor and not in PIE
		&& !GEditor->OnWorldDestroyed().IsBoundToObject(this))
	{
		// Editor pool manager instance has different lifetime than PIE pool manager instance,
		// So, to prevent memory leaks, clear all pools on switching levels in Editor
		TWeakObjectPtr<UPoolManager> WeakPoolManager(this);
		auto OnWorldDestroyed = [WeakPoolManager](UWorld* World)
		{
			if (!World || !World->IsEditorWorld()
				|| !GEditor || GEditor->IsPlaySessionInProgress())
			{
				// Not Editor world or in PIE
				return;
			}

			if (UPoolManager* PoolManager = WeakPoolManager.Get())
			{
				PoolManager->EmptyAllPools();
			}
		};

		GEditor->OnWorldDestroyed().AddWeakLambda(this, OnWorldDestroyed);
	}
#endif // WITH_EDITOR
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
