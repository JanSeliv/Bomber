// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/BmrActorUtilsLibrary.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrActorUtilsLibrary)

/*********************************************************************************************
 * Level Actors
 ********************************************************************************************* */

// Level Actors getter
void UBmrActorUtilsLibrary::GetLevelActors(FMapComponents& OutBitmaskedComponents, int32 ActorsTypesBitmask)
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	if (!GeneratedMap)
	{
		// Might be null if called before the map is initialized
		return;
	}

	if (!GeneratedMap->MapComponents.Num())
	{
		return;
	}

	if (!OutBitmaskedComponents.IsEmpty())
	{
		OutBitmaskedComponents.Empty();
	}

	for (UBmrMapComponent* MapComponentIt : GeneratedMap->MapComponents)
	{
		if (MapComponentIt
		    && MapComponentIt->IsActive()
		    && EnumHasAnyFlags(MapComponentIt->GetActorType(), TO_ENUM(EBmrActorType, ActorsTypesBitmask)))
		{
			OutBitmaskedComponents.Add(MapComponentIt);
		}
	}
}

// Returns level actors that are located on the specified cells
void UBmrActorUtilsLibrary::GetLevelActorsOnCells(FMapComponents& OutMapComponents, const FBmrCells& InCells)
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	if (!GeneratedMap)
	{
		// Might be null if called before the map is initialized
		return;
	}

	if (!GeneratedMap->MapComponents.Num())
	{
		return;
	}

	if (!OutMapComponents.IsEmpty())
	{
		OutMapComponents.Empty();
	}

	for (UBmrMapComponent* MapComponentIt : GeneratedMap->MapComponents)
	{
		const FBmrCell& CellIt = MapComponentIt && MapComponentIt->IsActive() ? MapComponentIt->GetCell() : FBmrCell::InvalidCell;
		if (CellIt.IsValid()
		    && InCells.Contains(CellIt))
		{
			OutMapComponents.Add(MapComponentIt);
		}
	}
}

// Returns the index comparing to actors of its type on the Generated Map
int32 UBmrActorUtilsLibrary::GetIndexByLevelActor(const UBmrMapComponent* InMapComponent)
{
	if (!ensureMsgf(InMapComponent, TEXT("ASSERT: [%i] %hs:\n'InMapComponent' is not valid!"), __LINE__, __FUNCTION__))
	{
		return INDEX_NONE;
	}

	// First try to get the index in Components array
	// It's more reliable for actors, since it contained in the order of creation
	FMapComponents MapComponents;
	int32 Index = 0;
	const EBmrActorType ActorType = InMapComponent->GetActorType();
	GetLevelActors(MapComponents, TO_FLAG(ActorType));
	for (const UBmrMapComponent* MapComponentIt : MapComponents)
	{
		if (MapComponentIt == InMapComponent)
		{
			return Index;
		}
		++Index;
	}

	return INDEX_NONE;
}

// Returns the level actor by its index: order of creation in the level
UBmrMapComponent* UBmrActorUtilsLibrary::GetLevelActorByIndex(int32 Index, int32 ActorsTypesBitmask)
{
	if (Index < 0)
	{
		return nullptr;
	}

	FMapComponents MapComponents;
	GetLevelActors(MapComponents, ActorsTypesBitmask);
	if (Index >= MapComponents.Num())
	{
		return nullptr;
	}

	int32 CurrentIndex = 0;
	for (UBmrMapComponent* MapComponentIt : MapComponents)
	{
		if (CurrentIndex == Index)
		{
			return MapComponentIt;
		}
		++CurrentIndex;
	}

	return nullptr;
}

// Takes level actors and returns only matching with specified actor types
FMapComponents UBmrActorUtilsLibrary::FilterLevelActors(const FMapComponents& InActors, int32 ActorsTypesBitmask)
{
	FMapComponents FilteredActors;
	for (UBmrMapComponent* It : InActors)
	{
		if (It
		    && EnumHasAnyFlags(It->GetActorType(), TO_ENUM(EBmrActorType, ActorsTypesBitmask)))
		{
			FilteredActors.Add(It);
		}
	}
	return FilteredActors;
}