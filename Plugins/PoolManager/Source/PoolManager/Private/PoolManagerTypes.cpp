// Copyright (c) Yevhenii Selivanov

#include "PoolManagerTypes.h"

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