// Copyright (c) Yevhenii Selivanov

#include "Structures/MapComponentsContainer.h"
//---
#include "Components/MapComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MapComponentsContainer)

FMapComponentSpec::FMapComponentSpec(UMapComponent& InMapComponent)
	: MapComponent(&InMapComponent)
	, Cell(InMapComponent.GetCell()) {}

FMapComponentSpec::FMapComponentSpec(FPoolObjectHandle InPoolObjectHandle)
	: PoolObjectHandle(MoveTemp(InPoolObjectHandle)) {}

void FMapComponentSpec::PreReplicatedRemove(const FMapComponentsContainer& InMapComponentsContainer)
{
	// On client, level actor removal was just replicated, perform cleanup to avoid unsynced cell data or dangling map component pointer

	Cell = FCell::InvalidCell;

	if (MapComponent)
	{
		MapComponent->OnPreRemoved();
		MapComponent->OnPostRemoved();
		MapComponent = nullptr;
	}
}

void FMapComponentSpec::PostReplicatedAdd(const FMapComponentsContainer& InMapComponentsContainer)
{
	// The level actor was added, update both the replicated cell and the map component

	if (IsValid())
	{
		MapComponent->SetCell(Cell);
		MapComponent->OnAdded();
	}
}

void FMapComponentSpec::PostReplicatedChange(const FMapComponentsContainer& InMapComponentsContainer)
{
	// The level actor was changed, update the replicated cell e.g: player character moved

	if (IsValid())
	{
		MapComponent->SetCell(Cell);
	}
}

FMapComponentsIterator::FMapComponentsIterator(const TArray<FMapComponentSpec>& InItems)
	: Items(InItems)
	, Index(0) {}

FMapComponentsIterator::FMapComponentsIterator(const TArray<FMapComponentSpec>& InItems, int32 StartIndex)
	: Items(InItems)
	, Index(StartIndex) {}

FMapComponentsIterator& FMapComponentsIterator::operator++()
{
	if (Items.IsValidIndex(Index + 1))
	{
		++Index;
	}
	else
	{
		Index = Items.Num(); // Set to 'end' position
	}
	return *this;
}

UMapComponent* FMapComponentsIterator::operator*() const
{
	const bool bValidIndex = Items.IsValidIndex(Index);
	checkf(bValidIndex, TEXT("ERROR: [%i] %s:\nIndex %i is not valid for array of Map Components with length of %i!"), __LINE__, *FString(__FUNCTION__), Index, Items.Num());
	return Items[Index].MapComponent;
}

// Marks this spec as dirty to push changes for replication, if valid
void FMapComponentsContainer::MarkItemDirty(FFastArraySerializerItem& Item)
{
	// First, make sure the spec is fully valid
	// Wait otherwise: it's often called when only Cell or Map Component is set (during construction), but push if only both are set
	FMapComponentSpec& Spec = static_cast<FMapComponentSpec&>(Item);
	if (Spec.IsValid())
	{
		FIrisFastArraySerializer::MarkItemDirty(Spec);
	}
}

FMapComponentSpec& FMapComponentsContainer::FindOrAdd(UMapComponent& MapComponent)
{
	if (FMapComponentSpec* FoundSpec = Find(&MapComponent))
	{
		return *FoundSpec;
	}

	FMapComponentSpec& AddedSpecRef = Items.Emplace_GetRef(MapComponent);
	AddedSpecRef.Cell = MapComponent.GetCell();
	MarkItemDirty(AddedSpecRef);
	return AddedSpecRef;
}

FMapComponentSpec& FMapComponentsContainer::FindOrAdd(const FPoolObjectHandle& PoolObjectHandle)
{
	checkf(PoolObjectHandle.IsValid(), TEXT("ERROR: [%i] %s:\n'PoolObjectHandle' is not valid!"), __LINE__, *FString(__FUNCTION__));

	if (FMapComponentSpec* FoundSpec = Find(PoolObjectHandle))
	{
		return *FoundSpec;
	}

	FMapComponentSpec& AddedSpecRef = Items.Emplace_GetRef(PoolObjectHandle);
	MarkItemDirty(AddedSpecRef);
	return AddedSpecRef;
}

void FMapComponentsContainer::Remove(const UMapComponent* MapComponent)
{
	if (const FMapComponentSpec* FoundSpec = Find(MapComponent))
	{
		// Remove first occurrence since there is only one Map Component
		const int8 bRemoved = Items.RemoveSingleSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Map Component: %s"), __LINE__, *FString(__FUNCTION__), *GetNameSafe(MapComponent));
		MarkArrayDirty();
	}
}

void FMapComponentsContainer::Remove(const FCell& Cell)
{
	if (const FMapComponentSpec* FoundSpec = Find(Cell))
	{
		const int8 bRemoved = Items.RemoveSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Cell: %s"), __LINE__, *FString(__FUNCTION__), *Cell.ToString());
		MarkArrayDirty();
	}
}

void FMapComponentsContainer::Remove(const FPoolObjectHandle& PoolObjectHandle)
{
	if (const FMapComponentSpec* FoundSpec = Find(PoolObjectHandle))
	{
		// Remove first occurrence since there is only one Handle
		const int8 bRemoved = Items.RemoveSingleSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Handle: %s"), __LINE__, *FString(__FUNCTION__), *PoolObjectHandle.GetHash().ToString());
		MarkArrayDirty();
	}
}