// Copyright (c) Yevhenii Selivanov

#include "DalRegistryRow.h"

// UE
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"

namespace DalRegistryRowInternal
{
	/** Shared implementation: gets all cached items from a registry */
	static bool GetAllCachedItems(const UDataRegistry* Registry, TMap<FDataRegistryId, const uint8*>& OutItems)
	{
		if (!Registry)
		{
			return false;
		}
		const UScriptStruct* ItemStruct = nullptr;
		Registry->GetAllCachedItems(OutItems, ItemStruct);
		return true;
	}
} // namespace DalRegistryRowInternal

// Finds the Data Registry whose ItemStruct matches InStruct, cached for fast repeated access
UDataRegistry* FDalRegistryRowAccessor::GetRegistryForStruct(const UScriptStruct* InStruct)
{
	if (!InStruct)
	{
		return nullptr;
	}

	// Check cache first
	static TMap<const UScriptStruct*, TWeakObjectPtr<UDataRegistry>> StructToRegistryCache;
	if (const TWeakObjectPtr<UDataRegistry>* Found = StructToRegistryCache.Find(InStruct))
	{
		if (UDataRegistry* CachedRegistry = Found->Get())
		{
			return CachedRegistry;
		}
		// Registry was garbage collected, remove stale entry
		StructToRegistryCache.Remove(InStruct);
	}

	// Reverse lookup: iterate all registries and find one with matching ItemStruct
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	if (!DRSubsystem)
	{
		return nullptr;
	}

	TArray<UDataRegistry*> AllRegistries;
	DRSubsystem->GetAllRegistries(AllRegistries);

	for (UDataRegistry* Registry : AllRegistries)
	{
		if (Registry && Registry->GetItemStruct() == InStruct)
		{
			StructToRegistryCache.Add(InStruct, Registry);
			return Registry;
		}
	}

	return nullptr;
}

// Returns total number of cached rows for the given row struct type, including child structs
int32 FDalRegistryRowAccessor::GetRowsNum(const UScriptStruct* InStruct)
{
	int32 Total = 0;
	ForEachRegistry(InStruct, [&Total](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		Total += CachedItems.Num();
	});
	return Total;
}

// Returns raw pointer to cached item data by struct type and RowName, O(1) lookup, or nullptr
const uint8* FDalRegistryRowAccessor::GetRowByName(const UScriptStruct* InStruct, FName RowName)
{
	const uint8* FoundRow = nullptr;
	ForEachRegistry(InStruct, [&FoundRow, RowName](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		if (FoundRow)
		{
			// First match already found, skip remaining registries
			return;
		}
		const uint8* OutItemMemory = nullptr;
		const UScriptStruct* OutItemStruct = nullptr;
		const FDataRegistryId ItemId(FDataRegistryType(Registry->GetRegistryType()), RowName);
		Registry->GetCachedItemRaw(OutItemMemory, OutItemStruct, ItemId);
		FoundRow = OutItemMemory;
	});
	return FoundRow;
}

// Returns raw pointer to cached item data at specified index for the given struct type, or nullptr
const uint8* FDalRegistryRowAccessor::GetRowByIndex(const UScriptStruct* InStruct, int32 Index)
{
	const uint8* FoundRow = nullptr;
	int32 RemainingIndex = Index;
	ForEachRegistry(InStruct, [&FoundRow, &RemainingIndex](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		if (FoundRow)
		{
			// Target index already resolved in earlier registry
			return;
		}
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		if (RemainingIndex >= CachedItems.Num())
		{
			// Index falls beyond this registry, carry remainder onward
			RemainingIndex -= CachedItems.Num();
			return;
		}
		int32 CurrentIndex = 0;
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			if (CurrentIndex == RemainingIndex)
			{
				FoundRow = Pair.Value;
				break;
			}
			++CurrentIndex;
		}
	});
	return FoundRow;
}

// Returns the row name at specified index for the given struct type, or NAME_None
FName FDalRegistryRowAccessor::GetRowNameByIndex(const UScriptStruct* InStruct, int32 Index)
{
	FName FoundName = NAME_None;
	bool bFound = false;
	int32 RemainingIndex = Index;
	ForEachRegistry(InStruct, [&FoundName, &bFound, &RemainingIndex](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		if (bFound)
		{
			// Target index already resolved in earlier registry
			return;
		}
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		if (RemainingIndex >= CachedItems.Num())
		{
			// Index falls beyond this registry, carry remainder onward
			RemainingIndex -= CachedItems.Num();
			return;
		}
		int32 CurrentIndex = 0;
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			if (CurrentIndex == RemainingIndex)
			{
				FoundName = Pair.Key.ItemName;
				bFound = true;
				break;
			}
			++CurrentIndex;
		}
	});
	return FoundName;
}

// Iterates all cached items for the given struct type, calling Callback with raw item data
void FDalRegistryRowAccessor::ForEachRow(const UScriptStruct* InStruct, const TFunctionRef<void(const uint8*)>& Callback)
{
	ForEachRegistry(InStruct, [&Callback](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			Callback(Pair.Value);
		}
	});
}

// Iterates all cached items for the given struct type, calling Callback with item name and raw data
void FDalRegistryRowAccessor::ForEachRowWithName(const UScriptStruct* InStruct, const TFunctionRef<void(FName ItemName, const uint8*)>& Callback)
{
	ForEachRegistry(InStruct, [&Callback](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			Callback(Pair.Key.ItemName, Pair.Value);
		}
	});
}

// Finds first cached item matching predicate, or nullptr
const uint8* FDalRegistryRowAccessor::GetRowByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate)
{
	const uint8* FoundRow = nullptr;
	ForEachRegistry(InStruct, [&FoundRow, &Predicate](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		if (FoundRow)
		{
			// First match already found, skip remaining registries
			return;
		}
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			if (Predicate(Pair.Value))
			{
				FoundRow = Pair.Value;
				break;
			}
		}
	});
	return FoundRow;
}

// Returns the row name of first cached item matching predicate, or NAME_None
FName FDalRegistryRowAccessor::GetRowNameByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate)
{
	FName FoundName = NAME_None;
	bool bFound = false;
	ForEachRegistry(InStruct, [&FoundName, &bFound, &Predicate](const UDataRegistry* Registry, const UScriptStruct* /*ItemStruct*/)
	{
		if (bFound)
		{
			// First match already found, skip remaining registries
			return;
		}
		TMap<FDataRegistryId, const uint8*> CachedItems;
		DalRegistryRowInternal::GetAllCachedItems(Registry, CachedItems);
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			if (Predicate(Pair.Value))
			{
				FoundName = Pair.Key.ItemName;
				bFound = true;
				break;
			}
		}
	});
	return FoundName;
}

// Returns this row's DR row name matched by reflection value-equality, or NAME_None
FName FDalRegistryRowAccessor::GetRowName(const UScriptStruct* InStruct, const uint8* RowData)
{
	if (!InStruct
	    || !RowData)
	{
		return NAME_None;
	}

	return GetRowNameByPredicate(InStruct, [InStruct, RowData](const uint8* CachedRowData)
	{
		return InStruct->CompareScriptStruct(CachedRowData, RowData, PPF_None);
	});
}

// Counts cached items matching predicate
int32 FDalRegistryRowAccessor::CountRowsByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate)
{
	int32 Count = 0;
	ForEachRow(InStruct, [&Count, &Predicate](const uint8* Data)
	{
		if (Predicate(Data))
		{
			++Count;
		}
	});
	return Count;
}

// Returns all Data Registries whose ItemStruct is a child of InBaseStruct
void FDalRegistryRowAccessor::ForEachRegistry(const UScriptStruct* InBaseStruct, const TFunctionRef<void(UDataRegistry* Registry, const UScriptStruct* ItemStruct)>& Callback)
{
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	if (!DRSubsystem)
	{
		// Registries unavailable during engine shutdown
		return;
	}

	TArray<UDataRegistry*> AllRegistries;
	DRSubsystem->GetAllRegistries(AllRegistries);
	for (UDataRegistry* Registry : AllRegistries)
	{
		const UScriptStruct* ItemStruct = Registry ? Registry->GetItemStruct() : nullptr;
		if (ItemStruct
		    && ItemStruct->IsChildOf(InBaseStruct))
		{
			Callback(Registry, ItemStruct);
		}
	}
}
