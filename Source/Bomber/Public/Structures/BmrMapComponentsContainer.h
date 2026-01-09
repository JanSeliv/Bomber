// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Iris/ReplicationState/IrisFastArraySerializer.h"

// Bomber
#include "BmrCell.h"
#include "Data/PoolObjectHandle.h"

// UE
#include "Engine/NetSerialization.h" // FVector_NetQuantize

#include "BmrMapComponentsContainer.generated.h"

/*********************************************************************************************
 * This file is designed to improve the replication of array of Map Components.
 * Using FFastArraySerializer in FBmrMapComponentsContainer allows for individual element tracking.
 * This way, the array  replicates accurately even if the number of elements remains the same,
 * overcoming Unreal Engine's limitation where OnRep may not trigger in such cases.
 *********************************************************************************************/

struct FBmrMapComponentsContainer;
struct FBmrCell;

class UBmrMapComponent;

/**
 * Represents a specification for a map component, inheriting from FFastArraySerializerItem
 * to take advantage of its serialization and network replication capabilities.
 * PreReplicatedRemove and PostReplicatedAdd can be added to handle custom logic.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrMapComponentSpec : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBmrMapComponentSpec() = default;
	FBmrMapComponentSpec(UBmrMapComponent& InMapComponent);
	FBmrMapComponentSpec(FPoolObjectHandle InPoolObjectHandle);

	/** The map component to be replicated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	TObjectPtr<UBmrMapComponent> MapComponent = nullptr;

	/** The position of the map component on the level.
	 * Replicated here instead of in the component to stay in sync with the array, avoiding component replication delay
	 * Uses NetQuantize to optimize network traffic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	FVector_NetQuantize Cell = FBmrCell::InvalidCell;

	/** Unique ID of Map Component's owner actor in the Pool Manager.
	 * Is useful to track the owner actor lifecycle even it is not spawned yet, but its Spawn Request is in queue.
	 * Is NOT replicated and exists only on the server side. */
	FPoolObjectHandle PoolObjectHandle = FPoolObjectHandle::EmptyHandle;

	/** Returns if current data is valid. If not, probably it's pending spawn or not replicated yet. */
	bool FORCEINLINE IsValid() const { return FBmrCell(Cell).IsValid() && MapComponent != nullptr; }

	/*********************************************************************************************
	 * FFastArraySerializerItem implementation
	 ********************************************************************************************* */

	void PreReplicatedRemove(const FBmrMapComponentsContainer& InMapComponentsContainer);
	void PostReplicatedAdd(const FBmrMapComponentsContainer& InMapComponentsContainer);
	void PostReplicatedChange(const FBmrMapComponentsContainer& InMapComponentsContainer);

	/*********************************************************************************************
	 * Convenience operators to treat FBmrMapComponentSpec as a UBmrMapComponent*
	 ********************************************************************************************* */

	friend BOMBER_API bool operator==(const FBmrMapComponentSpec& A, const FBmrMapComponentSpec& B) { return A.MapComponent == B.MapComponent && A.Cell == B.Cell && A.PoolObjectHandle == B.PoolObjectHandle; }
	friend BOMBER_API bool operator==(const FBmrMapComponentSpec& A, const UBmrMapComponent* B) { return A.MapComponent == B; }
	friend BOMBER_API bool operator==(const FBmrMapComponentSpec& A, const FBmrCell& B) { return FBmrCell(A.Cell) == B; }
	friend BOMBER_API bool operator==(const FBmrMapComponentSpec& A, const FPoolObjectHandle& B) { return A.PoolObjectHandle == B; }
};

/**
 * Iterator structure for FBmrMapComponentsContainer, designed to simplify the traversal of the container's items.
 * Facilitates cleaner and safer access to elements.
 */
struct BOMBER_API FBmrMapComponentsIterator
{
private:
	const TArray<FBmrMapComponentSpec>& Items;
	int32 Index;

public:
	FBmrMapComponentsIterator(const TArray<FBmrMapComponentSpec>& InItems);
	FBmrMapComponentsIterator(const TArray<FBmrMapComponentSpec>& InItems, int32 StartIndex);

	FBmrMapComponentsIterator& operator++();

	bool FORCEINLINE operator!=(const FBmrMapComponentsIterator& Other) const { return Index != Other.Index; }

	UBmrMapComponent* operator*() const;
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
struct BOMBER_API FBmrMapComponentsContainer : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	/** Internal token for tracking replication progress, is not replicated, but is incremented locally on each instance whenever any level actor is spawned. */
	UPROPERTY(Transient, NotReplicated)
	int32 LocalReplicationToken = 0;

	/** The main data array for replication.
	 * @warning It shouldn't be accessed directly, use UBmrActorUtilsLibrary functions instead for obtaining Level Actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	TArray<FBmrMapComponentSpec> Items;

	/*********************************************************************************************
	 * FFastArraySerializer implementation
	 ********************************************************************************************* */

	/** Custom delta serialization for the map components array. Enables network transmission of changes. */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms) { return FastArrayDeltaSerialize<FBmrMapComponentSpec, FBmrMapComponentsContainer>(Items, DeltaParms, *this); }

	/** Checks if an item should be written during delta serialization, considering client or server context. */
	template <typename Type, typename SerializerType>
	static bool ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient);

	/** Marks this spec as dirty to push changes for replication, if valid. */
	void MarkItemDirty(FFastArraySerializerItem& Item);

	/*********************************************************************************************
	 * Convenience methods to treat FBmrMapComponentsContainer as a TArray<UBmrMapComponent*>
	 ********************************************************************************************* */
public:
	/** Iterators to for-each loop through the Items. */
	FBmrMapComponentsIterator begin() const { return FBmrMapComponentsIterator(Items); }
	FBmrMapComponentsIterator end() const { return FBmrMapComponentsIterator(Items, Items.Num()); }

	FORCEINLINE int32 Num() const { return Items.Num(); }

	FORCEINLINE bool Contains(const UBmrMapComponent* Item) const { return Items.Contains(Item); }
	FORCEINLINE bool Contains(const FBmrCell& Cell) const { return Items.Contains(Cell); }
	FORCEINLINE bool Contains(const FPoolObjectHandle& PoolObjectHandle) const { return Items.Contains(PoolObjectHandle); }
	FORCEINLINE bool ContainsByPredicate(const TFunctionRef<bool(const FBmrMapComponentSpec&)>& Predicate) const { return Items.ContainsByPredicate(Predicate); }

	FBmrMapComponentSpec* Find(const UBmrMapComponent* Item) { return Items.FindByKey(Item); }
	FBmrMapComponentSpec* Find(const FBmrCell& Cell) { return Cell.IsValid() ? Items.FindByKey(Cell) : nullptr; }
	FBmrMapComponentSpec* Find(const FPoolObjectHandle& PoolObjectHandle) { return Items.FindByKey(PoolObjectHandle); }

	FBmrMapComponentSpec& FindOrAdd(UBmrMapComponent& MapComponent);
	FBmrMapComponentSpec& FindOrAdd(const FPoolObjectHandle& PoolObjectHandle);

	void Remove(const UBmrMapComponent* MapComponent);
	void Remove(const FBmrCell& Cell);
	void Remove(const FPoolObjectHandle& PoolObjectHandle);

	FORCEINLINE bool IsValidIndex(int32 Index) const { return Items.IsValidIndex(Index); }

	UBmrMapComponent* operator[](const int32 Index) const { return IsValidIndex(Index) ? Items[Index].MapComponent : nullptr; }
};

/**
 * Specialization of TStructOpsTypeTraits for FBmrMapComponentsContainer.
 * Enables network delta serialization for efficient and reliable replication.
 */
template <>
struct BOMBER_API TStructOpsTypeTraits<FBmrMapComponentsContainer> : public TStructOpsTypeTraitsBase2<FBmrMapComponentsContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

template <typename Type, typename SerializerType>
bool FBmrMapComponentsContainer::ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient)
{
	if (bIsWritingOnClient)
	{
		return Item.ReplicationID != INDEX_NONE;
	}

	return true;
}