// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/LevelActorsUtilsLibrary.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(LevelActorsUtilsLibrary)

// Level Actors getter
void ULevelActorsUtilsLibrary::GetLevelActors(FMapComponents& OutBitmaskedComponents, int32 ActorsTypesBitmask)
{
	const AGeneratedMap* GeneratedMap = AGeneratedMap::GetGeneratedMap();
	if (!ensureMsgf(GeneratedMap, TEXT("ASSERT: [%i] %hs:\n'GeneratedMap' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (!GeneratedMap->MapComponentsInternal.Num())
	{
		return;
	}

	for (UMapComponent* MapComponentIt : GeneratedMap->MapComponentsInternal)
	{
		if (MapComponentIt
		    && EnumHasAnyFlags(MapComponentIt->GetActorType(), TO_ENUM(EActorType, ActorsTypesBitmask)))
		{
			OutBitmaskedComponents.Add(MapComponentIt);
		}
	}
}

// Returns the index comparing to actors of its type on the Generated Map
int32 ULevelActorsUtilsLibrary::GetIndexByLevelActor(const UMapComponent* InMapComponent)
{
	if (!ensureMsgf(InMapComponent, TEXT("ASSERT: [%i] %hs:\n'InMapComponent' is not valid!"), __LINE__, __FUNCTION__))
	{
		return INDEX_NONE;
	}

	// First try to get the index in Components array
	// It's more reliable for actors, since it contained in the order of creation 
	FMapComponents MapComponents;
	int32 Index = 0;
	const EActorType ActorType = InMapComponent->GetActorType();
	GetLevelActors(MapComponents, TO_FLAG(ActorType));
	for (const UMapComponent* MapComponentIt : MapComponents)
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
UMapComponent* ULevelActorsUtilsLibrary::GetLevelActorByIndex(int32 Index, int32 ActorsTypesBitmask)
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
	for (UMapComponent* MapComponentIt : MapComponents)
	{
		if (CurrentIndex == Index)
		{
			return MapComponentIt;
		}
		++CurrentIndex;
	}

	return nullptr;
}