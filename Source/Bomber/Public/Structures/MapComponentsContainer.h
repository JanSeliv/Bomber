// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Iris/ReplicationState/IrisFastArraySerializer.h"
//---
#include "Cell.h"
#include "PoolManagerTypes.h" // FPoolObjectHandle
#include "Engine/NetSerialization.h" // FVector_NetQuantize
//---
#include "MapComponentsContainer.generated.h"

/*********************************************************************************************
 * This file is designed to improve the replication of array of Map Components.
 * Using FFastArraySerializer in FMapComponentsContainer allows for individual element tracking.
 * This way, the array  replicates accurately even if the number of elements remains the same,
 * overcoming Unreal Engine's limitation where OnRep may not trigger in such cases.
 *********************************************************************************************/

struct FMapComponentsContainer;
struct FCell;

class UMapComponent;

/**
 * Represents a specification for a map component, inheriting from FFastArraySerializerItem 
 * to take advantage of its serialization and network replication capabilities.
 * PreReplicatedRemove and PostReplicatedAdd can be added to handle custom logic.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FMapComponentSpec : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FMapComponentSpec() = default;
	FMapComponentSpec(UMapComponent& InMapComponent);
	FMapComponentSpec(FPoolObjectHandle InPoolObjectHandle);

	/** The map component to be replicated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TObjectPtr<UMapComponent> MapComponent = nullptr;

	/** The position of the map component on the level.
	 * Replicated here instead of in the component to stay in sync with the array, avoiding component replication delay
	 * Uses NetQuantize to optimize network traffic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FVector_NetQuantize Cell = FCell::InvalidCell;

	/** Unique ID of Map Component's owner actor in the Pool Manager.
	 * Is useful to track the owner actor lifecycle even it is not spawned yet, but its Spawn Request is in queue.
	 * Is NOT replicated and exists only on the server side. */
	FPoolObjectHandle PoolObjectHandle = FPoolObjectHandle::EmptyHandle;

	/** Returns if current data is valid. If not, probably it's pending spawn or not replicated yet. */
	bool FORCEINLINE IsValid() const { return FCell(Cell).IsValid() && MapComponent != nullptr; }

	/*********************************************************************************************
	 * FFastArraySerializerItem implementation
	 ********************************************************************************************* */

	void PreReplicatedRemove(const FMapComponentsContainer& InMapComponentsContainer);
	void PostReplicatedAdd(const FMapComponentsContainer& InMapComponentsContainer);
	void PostReplicatedChange(const FMapComponentsContainer& InMapComponentsContainer);

	/*********************************************************************************************
	 * Convenience operators to treat FMapComponentSpec as a UMapComponent*
	 ********************************************************************************************* */

	friend BOMBER_API bool operator==(const FMapComponentSpec& A, const FMapComponentSpec& B) { return A.MapComponent == B.MapComponent && A.Cell == B.Cell && A.PoolObjectHandle == B.PoolObjectHandle; }
	friend BOMBER_API bool operator==(const FMapComponentSpec& A, const UMapComponent* B) { return A.MapComponent == B; }
	friend BOMBER_API bool operator==(const FMapComponentSpec& A, const FCell& B) { return A.Cell == B; }
	friend BOMBER_API bool operator==(const FMapComponentSpec& A, const FPoolObjectHandle& B) { return A.PoolObjectHandle == B; }
};

/**
 * Iterator structure for FMapComponentsContainer, designed to simplify the traversal of the container's items.
 * Facilitates cleaner and safer access to elements.
 */
struct BOMBER_API FMapComponentsIterator
{
private:
	const TArray<FMapComponentSpec>& Items;
	int32 Index;

public:
	FMapComponentsIterator(const TArray<FMapComponentSpec>& InItems);
	FMapComponentsIterator(const TArray<FMapComponentSpec>& InItems, int32 StartIndex);

	FMapComponentsIterator& operator++();

	bool FORCEINLINE operator!=(const FMapComponentsIterator& Other) const { return Index != Other.Index; }

	UMapComponent* operator*() const;
};

/**
 * Is exposed to replicate the Map Components and their cells.
 * Utilizes the fast array serialization instead of regular array of objects for next reasons:
 * - reliable network replication even when the number of components in the array remains unchanged
 * - minimizes bandwidth usage, essential as it can be large and frequently changing
 * - efficiently tracks changes, including additions and removals
 * - ensures each component and its cell replicate together as one package due to its structured design
 */
USTRUCT(BlueprintType)
struct BOMBER_API FMapComponentsContainer : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	/** Internal token for tracking replication progress, is not replicated, but is incremented locally on each instance whenever any level actor is spawned. */
	UPROPERTY(Transient, NotReplicated)
	int32 LocalReplicationToken = 0;

	/** The main data array for replication.
	 * @warning It shouldn't be accessed directly, use ULevelActorsUtilsLibrary functions instead for obtaining Level Actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TArray<FMapComponentSpec> Items;

	/*********************************************************************************************
	 * FFastArraySerializer implementation
	 ********************************************************************************************* */

	/** Custom delta serialization for the map components array. Enables network transmission of changes. */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms) { return FastArrayDeltaSerialize<FMapComponentSpec, FMapComponentsContainer>(Items, DeltaParms, *this); }

	/** Checks if an item should be written during delta serialization, considering client or server context. */
	template <typename Type, typename SerializerType>
	static bool ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient);

	/** Marks this spec as dirty to push changes for replication, if valid. */
	void MarkItemDirty(FFastArraySerializerItem& Item);

	/*********************************************************************************************
	 * Convenience methods to treat FMapComponentsContainer as a TArray<UMapComponent*>
	 ********************************************************************************************* */
public:
	/** Iterators to for-each loop through the Items. */
	FMapComponentsIterator begin() const { return FMapComponentsIterator(Items); }
	FMapComponentsIterator end() const { return FMapComponentsIterator(Items, Items.Num()); }

	FORCEINLINE int32 Num() const { return Items.Num(); }

	FORCEINLINE bool Contains(const UMapComponent* Item) const { return Items.Contains(Item); }
	FORCEINLINE bool Contains(const FCell& Cell) const { return Items.Contains(Cell); }
	FORCEINLINE bool Contains(const FPoolObjectHandle& PoolObjectHandle) const { return Items.Contains(PoolObjectHandle); }
	FORCEINLINE bool ContainsByPredicate(const TFunctionRef<bool(const FMapComponentSpec&)>& Predicate) const { return Items.ContainsByPredicate(Predicate); }

	FMapComponentSpec* Find(const UMapComponent* Item) { return Items.FindByKey(Item); }
	FMapComponentSpec* Find(const FCell& Cell) { return Cell.IsValid() ? Items.FindByKey(Cell) : nullptr; }
	FMapComponentSpec* Find(const FPoolObjectHandle& PoolObjectHandle) { return Items.FindByKey(PoolObjectHandle); }

	FMapComponentSpec& FindOrAdd(UMapComponent& MapComponent);
	FMapComponentSpec& FindOrAdd(const FPoolObjectHandle& PoolObjectHandle);

	void Remove(const UMapComponent* MapComponent);
	void Remove(const FCell& Cell);
	void Remove(const FPoolObjectHandle& PoolObjectHandle);

	FORCEINLINE bool IsValidIndex(int32 Index) const { return Items.IsValidIndex(Index); }

	UMapComponent* operator[](const int32 Index) const { return IsValidIndex(Index) ? Items[Index].MapComponent : nullptr; }
};

/**
 * Specialization of TStructOpsTypeTraits for FMapComponentsContainer.
 * Enables network delta serialization for efficient and reliable replication.
 */
template <>
struct BOMBER_API TStructOpsTypeTraits<FMapComponentsContainer> : public TStructOpsTypeTraitsBase2<FMapComponentsContainer>
{
	enum { WithNetDeltaSerializer = true };
};

template <typename Type, typename SerializerType>
bool FMapComponentsContainer::ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient)
{
	if (bIsWritingOnClient)
	{
		return Item.ReplicationID != INDEX_NONE;
	}

	return true;
}