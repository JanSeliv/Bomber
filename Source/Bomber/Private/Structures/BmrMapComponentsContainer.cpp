// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrMapComponentsContainer.h"

// Bomber
#include "Components/BmrMapComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMapComponentsContainer)

FBmrMapComponentSpec::FBmrMapComponentSpec(UBmrMapComponent& InMapComponent)
    : MapComponent(&InMapComponent)
    , Cell(InMapComponent.GetCell())
{
}

FBmrMapComponentSpec::FBmrMapComponentSpec(FPoolObjectHandle InPoolObjectHandle)
    : PoolObjectHandle(MoveTemp(InPoolObjectHandle))
{
}

void FBmrMapComponentSpec::PreReplicatedRemove(const FBmrMapComponentsContainer& InMapComponentsContainer)
{
	// On client, level actor removal was just replicated, perform cleanup to avoid unsynced cell data or dangling map component pointer

	Cell = FBmrCell::InvalidCell;

	if (MapComponent)
	{
		MapComponent->OnPreRemoved();
		MapComponent->OnPostRemoved();
		MapComponent = nullptr;
	}
}

void FBmrMapComponentSpec::PostReplicatedAdd(const FBmrMapComponentsContainer& InMapComponentsContainer)
{
	// The level actor was added, update both the replicated cell and the map component

	if (IsValid())
	{
		MapComponent->SetCell(Cell);
		MapComponent->OnAdded();
	}
}

void FBmrMapComponentSpec::PostReplicatedChange(const FBmrMapComponentsContainer& InMapComponentsContainer)
{
	// The level actor was changed, update the replicated cell e.g: player character moved

	if (IsValid())
	{
		MapComponent->SetCell(Cell);
	}
}

FBmrMapComponentsIterator::FBmrMapComponentsIterator(const TArray<FBmrMapComponentSpec>& InItems)
    : Items(InItems)
    , Index(0)
{
}

FBmrMapComponentsIterator::FBmrMapComponentsIterator(const TArray<FBmrMapComponentSpec>& InItems, int32 StartIndex)
    : Items(InItems)
    , Index(StartIndex)
{
}

FBmrMapComponentsIterator& FBmrMapComponentsIterator::operator++()
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

UBmrMapComponent* FBmrMapComponentsIterator::operator*() const
{
	const bool bValidIndex = Items.IsValidIndex(Index);
	checkf(bValidIndex, TEXT("ERROR: [%i] %s:\nIndex %i is not valid for array of Map Components with length of %i!"), __LINE__, *FString(__FUNCTION__), Index, Items.Num());
	return Items[Index].MapComponent;
}

// Marks this spec as dirty to push changes for replication, if valid
void FBmrMapComponentsContainer::MarkItemDirty(FFastArraySerializerItem& Item)
{
	// First, make sure the spec is fully valid
	// Wait otherwise: it's often called when only Cell or Map Component is set (during construction), but push if only both are set
	FBmrMapComponentSpec& Spec = static_cast<FBmrMapComponentSpec&>(Item);
	if (Spec.IsValid())
	{
		FIrisFastArraySerializer::MarkItemDirty(Spec);
	}
}

FBmrMapComponentSpec& FBmrMapComponentsContainer::FindOrAdd(UBmrMapComponent& MapComponent)
{
	if (FBmrMapComponentSpec* FoundSpec = Find(&MapComponent))
	{
		return *FoundSpec;
	}

	FBmrMapComponentSpec& AddedSpecRef = Items.Emplace_GetRef(MapComponent);
	AddedSpecRef.Cell = MapComponent.GetCell();
	MarkItemDirty(AddedSpecRef);
	return AddedSpecRef;
}

FBmrMapComponentSpec& FBmrMapComponentsContainer::FindOrAdd(const FPoolObjectHandle& PoolObjectHandle)
{
	checkf(PoolObjectHandle.IsValid(), TEXT("ERROR: [%i] %s:\n'PoolObjectHandle' is not valid!"), __LINE__, *FString(__FUNCTION__));

	if (FBmrMapComponentSpec* FoundSpec = Find(PoolObjectHandle))
	{
		return *FoundSpec;
	}

	FBmrMapComponentSpec& AddedSpecRef = Items.Emplace_GetRef(PoolObjectHandle);
	MarkItemDirty(AddedSpecRef);
	return AddedSpecRef;
}

void FBmrMapComponentsContainer::Remove(const UBmrMapComponent* MapComponent)
{
	if (const FBmrMapComponentSpec* FoundSpec = Find(MapComponent))
	{
		// Remove first occurrence since there is only one Map Component
		const int8 bRemoved = Items.RemoveSingleSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Map Component: %s"), __LINE__, *FString(__FUNCTION__), *GetNameSafe(MapComponent));
		MarkArrayDirty();
	}
}

void FBmrMapComponentsContainer::Remove(const FBmrCell& Cell)
{
	if (const FBmrMapComponentSpec* FoundSpec = Find(Cell))
	{
		const int8 bRemoved = Items.RemoveSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Cell: %s"), __LINE__, *FString(__FUNCTION__), *Cell.ToString());
		MarkArrayDirty();
	}
}

void FBmrMapComponentsContainer::Remove(const FPoolObjectHandle& PoolObjectHandle)
{
	if (const FBmrMapComponentSpec* FoundSpec = Find(PoolObjectHandle))
	{
		// Remove first occurrence since there is only one Handle
		const int8 bRemoved = Items.RemoveSingleSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Handle: %s"), __LINE__, *FString(__FUNCTION__), *PoolObjectHandle.GetHash().ToString());
		MarkArrayDirty();
	}
}