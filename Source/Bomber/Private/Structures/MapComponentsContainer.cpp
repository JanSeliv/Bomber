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

FMapComponentSpec::FMapComponentSpec(UMapComponent* InMapComponent)
	: MapComponent(InMapComponent) {}

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

FMapComponentSpec& FMapComponentsContainer::Emplace(UMapComponent* MapComponent)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %s:\n'MapComponent' is null!"), __LINE__, *FString(__FUNCTION__));
	FMapComponentSpec& AddedSpecRef = Items.Emplace_GetRef(MapComponent);
	AddedSpecRef.Cell = MapComponent->GetCell();
	MarkItemDirty(AddedSpecRef);
	return AddedSpecRef;
}

void FMapComponentsContainer::Remove(UMapComponent* MapComponent)
{
	Items.Remove(MapComponent);
	MarkArrayDirty();
}
