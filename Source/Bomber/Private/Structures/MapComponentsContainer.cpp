// Copyright (c) Yevhenii Selivanov

#include "Structures/MapComponentsContainer.h"
//---
#include "Components/MapComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MapComponentsContainer)

bool operator==(const FMapComponentSpec& A, const FCell& B)
{
	const FCell& Cell = A.MapComponent ? A.MapComponent->GetCell() : FCell::InvalidCell;
	return Cell == B;
}

FMapComponentSpec::FMapComponentSpec(UMapComponent& InMapComponent)
	: MapComponent(&InMapComponent)
	, Cell(InMapComponent.GetCell()) {}

FMapComponentSpec::FMapComponentSpec(FPoolObjectHandle InPoolObjectHandle)
	: PoolObjectHandle(MoveTemp(InPoolObjectHandle)) {}

// Updates the cell of the map component according current data
void FMapComponentSpec::UpdateCellInComponent()
{
	if (MapComponent)
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

	// No need to set Cell or replicate it since there is no Map Component yet 
	return Items.Emplace_GetRef(PoolObjectHandle);
}

void FMapComponentsContainer::Remove(const UMapComponent* MapComponent)
{
	const FMapComponentSpec* FoundSpec = Find(MapComponent);
	if (ensureMsgf(FoundSpec, TEXT("ASSERT: [%i] %s:\n'FoundSpec' condition is FALSE"), __LINE__, *FString(__FUNCTION__)))
	{
		// Remove first occurrence since there is only one Map Component
		const int8 bRemoved = Items.RemoveSingleSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Map Component: %s"), __LINE__, *FString(__FUNCTION__), *GetNameSafe(MapComponent));
		MarkArrayDirty();
	}
}

void FMapComponentsContainer::Remove(const FCell& Cell)
{
	const FMapComponentSpec* FoundSpec = Find(Cell);
	if (ensureMsgf(FoundSpec, TEXT("ASSERT: [%i] %s:\n'FoundSpec' condition is FALSE"), __LINE__, *FString(__FUNCTION__)))
	{
		const int8 bRemoved = Items.RemoveSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Cell: %s"), __LINE__, *FString(__FUNCTION__), *Cell.ToString());
		MarkArrayDirty();
	}
}

void FMapComponentsContainer::Remove(const FPoolObjectHandle& PoolObjectHandle)
{
	const FMapComponentSpec* FoundSpec = Find(PoolObjectHandle);
	if (ensureMsgf(FoundSpec, TEXT("ASSERT: [%i] %s:\n'FoundSpec' condition is FALSE"), __LINE__, *FString(__FUNCTION__)))
	{
		// Remove first occurrence since there is only one Handle
		const int8 bRemoved = Items.RemoveSingleSwap(*FoundSpec);
		checkf(bRemoved, TEXT("ERROR: [%i] %s:\nFailed to remove next Handle: %s"), __LINE__, *FString(__FUNCTION__), *PoolObjectHandle.GetHash().ToString());
		// MarkArrayDirty() is skipped since there is no spawned Map Component yet
	}
}
