// Copyright (c) Yevhenii Selivanov

#include "PoolManagerTypes.h"

// Empty pool object data
const FPoolObjectData FPoolObjectData::EmptyObject = FPoolObjectData();

// Empty pool data container
const FPoolContainer FPoolContainer::EmptyPool = FPoolContainer();

// Parameterized constructor that takes object to keep
FPoolObjectData::FPoolObjectData(UObject* InPoolObject)
{
	PoolObject = InPoolObject;
}

// Parameterized constructor that takes a class of the pool
FPoolContainer::FPoolContainer(const UClass* InClass)
{
	ClassInPool = InClass;
}

// Returns the pointer to the Pool element by specified object
FPoolObjectData* FPoolContainer::FindInPool(const UObject* Object)
{
	if (!Object)
	{
		return nullptr;
	}

	return PoolObjects.FindByPredicate([Object](const FPoolObjectData& It)
	{
		return It.PoolObject == Object;
	});
}