// Copyright (c) Yevhenii Selivanov

#include "DalRegistrySubsystem.h"

// DAL
#include "DalRegistryRow.h"
#include "DalUtilsLibrary.h"

// UE
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DalRegistrySubsystem)

// Returns this Subsystem, is checked and will crash if can't be obtained
UDalRegistrySubsystem& UDalRegistrySubsystem::Get()
{
	UDalRegistrySubsystem* Subsystem = GetDalRegistrySubsystem();
	checkf(Subsystem, TEXT("ASSERT: [%i] %hs:\n'Subsystem' is null"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this Subsystem, nullptr if engine is not available (e.g. during shutdown)
UDalRegistrySubsystem* UDalRegistrySubsystem::GetDalRegistrySubsystem()
{
	return GEngine ? GEngine->GetEngineSubsystem<UDalRegistrySubsystem>() : nullptr;
}

/*********************************************************************************************
 * Load
 ********************************************************************************************* */

// Blueprint-callable wrapper for BindAndLoad
void UDalRegistrySubsystem::BPBindAndLoad(UObject* Owner, const UScriptStruct* RowStruct, const FOnDalRegistryRowsChanged& OnChanged)
{
	BindInternal(RowStruct, Owner, TDelegate<void()>::CreateLambda([OnChanged]()
	{
		OnChanged.ExecuteIfBound();
	}));
}

// Unsubscribes the given owner from all bound Data Registries, cancels pending async load, resets binding
void UDalRegistrySubsystem::UnbindFromDataRegistryLoad(const UObject* Owner)
{
	FDalRegistryBinding* Binding = OwnerBindings.Find(Owner);
	if (!Binding)
	{
		return;
	}

	if (Binding->StreamableHandle)
	{
		Binding->StreamableHandle->CancelHandle();
		Binding->StreamableHandle.Reset();
	}

	for (const UScriptStruct* StructIt : Binding->BoundStructs)
	{
		if (Owner)
		{
			UnbindOnRegistryChanged(StructIt, Owner);
		}
	}

	if (Owner)
	{
		if (UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get())
		{
			DRSubsystem->OnSubsystemInitialized().RemoveAll(Owner);
		}
	}

	OwnerBindings.Remove(Owner);
}

// Returns true if the given owner is currently bound to at least one Data Registry
bool UDalRegistrySubsystem::IsBound(const UObject* Owner) const
{
	const FDalRegistryBinding* Binding = OwnerBindings.Find(Owner);
	return Binding && !Binding->BoundStructs.IsEmpty();
}

// Returns true if all soft references for the given owner have finished async loading at least once
bool UDalRegistrySubsystem::IsLoaded(const UObject* Owner) const
{
	const FDalRegistryBinding* Binding = OwnerBindings.Find(Owner);
	return Binding && Binding->bIsLoaded;
}

// Re-gathers all soft paths for the given owner from all bound sources and starts async load
void UDalRegistrySubsystem::TryLoad(const UObject* Owner)
{
	if (!Owner)
	{
		return;
	}

	FDalRegistryBinding* Binding = OwnerBindings.Find(Owner);
	if (!Binding)
	{
		return;
	}

	// Cancel previous load before starting new one
	if (Binding->StreamableHandle)
	{
		Binding->StreamableHandle->CancelHandle();
		Binding->StreamableHandle.Reset();
	}

	TArray<FSoftObjectPath> PathsToLoad;
	for (const UScriptStruct* StructIt : Binding->BoundStructs)
	{
		GatherAllSoftPaths(StructIt, PathsToLoad);
	}

	const TWeakObjectPtr WeakOwner(Owner);
	auto FireLoadedOwner = [WeakOwner]()
	{
		UDalRegistrySubsystem* Subsystem = GetDalRegistrySubsystem();
		FDalRegistryBinding* DeferredBinding = Subsystem ? Subsystem->OwnerBindings.Find(WeakOwner) : nullptr;
		if (!DeferredBinding
		    || UDalUtilsLibrary::IsOwnerWorldStale(WeakOwner.Get()))
		{
			return;
		}

		DeferredBinding->bIsLoaded = true;
		for (const UScriptStruct* StructIt : DeferredBinding->BoundStructs)
		{
			Subsystem->NotifyPendingRowListeners(StructIt);
		}
		DeferredBinding->ChangedCallback.ExecuteIfBound();
	};

	if (PathsToLoad.IsEmpty())
	{
		UDalUtilsLibrary::ExecutePIESafe(Owner, MoveTemp(FireLoadedOwner));
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	Binding->StreamableHandle = StreamableManager.RequestAsyncLoad(MoveTemp(PathsToLoad), FStreamableDelegate::CreateLambda([WeakOwner, Callback = MoveTemp(FireLoadedOwner)]()
	{
		UDalUtilsLibrary::ExecutePIESafe(WeakOwner.Get(), Callback);
	}));
}

/*********************************************************************************************
 * Listeners
 ********************************************************************************************* */

// Blueprint-callable wrapper for ListenForDataRegistryRow
void UDalRegistrySubsystem::BPListenForDataRegistryRow(UObject* Owner, const FDataRegistryId& ItemId, const FOnDalRegistryRowLoaded& Completed)
{
	const UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	const UDataRegistry* Registry = DataRegistrySubsystem ? DataRegistrySubsystem->GetRegistryForType(ItemId.RegistryType.GetName()) : nullptr;
	const UScriptStruct* RowStruct = Registry ? Registry->GetItemStruct() : nullptr;
	const FName RowName = ItemId.ItemName;
	if (!ensureMsgf(RowStruct, TEXT("ASSERT: [%i] %hs:\n'RowStruct' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(RowName.IsValid(), TEXT("ASSERT: [%i] %hs:\n'RowName' is None!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	ListenForDataRegistryRowInternal(Owner, RowStruct, RowName, [Completed, RowName](const uint8* /*RowData*/)
	{
		Completed.ExecuteIfBound(RowName);
	});
}

// Runtime-struct lambda variant for polymorphic bases where the actual derived row type is only known at runtime, lifetime guaranteed by Internal
void UDalRegistrySubsystem::ListenForDataRegistryRow(UObject* Object, const UScriptStruct* InStruct, FName RowName, TFunction<void(FName)>&& Callback)
{
	ListenForDataRegistryRowInternal(Object, InStruct, RowName, [RowName, Callback = MoveTemp(Callback)](const uint8* /*RowData*/)
	{
		Callback(RowName);
	});
}

// Non-template implementation: queues a one-shot listener for (InStruct, RowName), fires immediately if row is already available
void UDalRegistrySubsystem::ListenForDataRegistryRowInternal(UObject* Owner, const UScriptStruct* InStruct, FName RowName, TFunction<void(const uint8*)>&& Callback)
{
	if (!ensureMsgf(Owner, TEXT("ASSERT: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(InStruct, TEXT("ASSERT: [%i] %hs:\n'InStruct' is null!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!RowName.IsNone(), TEXT("ASSERT: [%i] %hs:\n'RowName' is None!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	WaitForRegistries(Owner, [WeakOwner = TWeakObjectPtr(Owner), InStruct, RowName, Callback = MoveTemp(Callback)]() mutable
	{
		UObject* StrongOwner = WeakOwner.Get();
		if (!StrongOwner)
		{
			return;
		}

		// Try immediate resolution: row is present in the cache and all its soft refs are loaded
		if (const uint8* RowData = FDalRegistryRow::GetRowByName(InStruct, RowName))
		{
			if (AreSoftRefsLoadedForRow(InStruct, RowData))
			{
				Callback(RowData);
				return;
			}
		}

		// Passive listener: wait for a BindAndLoad elsewhere to drive loading and notify via NotifyPendingRowListeners
		Get().PendingRowListeners.Add({WeakOwner, RowName}, {InStruct, MoveTemp(Callback)});
	});
}

// Notifies all pending listeners observing the given struct, fires those whose rows are now ready
void UDalRegistrySubsystem::NotifyPendingRowListeners(const UScriptStruct* InStruct)
{
	TArray<FDalRegistryRowListenerKey> KeysForStruct;
	for (const TPair<FDalRegistryRowListenerKey, FDalRegistryRowListener>& Entry : PendingRowListeners)
	{
		if (Entry.Value.WeakStruct == InStruct)
		{
			KeysForStruct.Add(Entry.Key);
		}
	}

	for (const FDalRegistryRowListenerKey& Key : KeysForStruct)
	{
		TryFireAndRemoveListener(Key, InStruct);
	}
}

// Fires and removes the identified listener if its row is now resolvable, silently prunes entries whose owner has died
void UDalRegistrySubsystem::TryFireAndRemoveListener(const FDalRegistryRowListenerKey& Key, const UScriptStruct* InStruct)
{
	FDalRegistryRowListener* Listener = PendingRowListeners.Find(Key);
	if (!Listener)
	{
		return;
	}

	// Silently prune stale owners
	if (!Key.WeakOwner.IsValid())
	{
		PendingRowListeners.Remove(Key);
		return;
	}

	const uint8* RowData = FDalRegistryRow::GetRowByName(InStruct, Key.RowName);
	if (!RowData
	    || !AreSoftRefsLoadedForRow(InStruct, RowData))
	{
		return;
	}

	// Extract before Remove to avoid dangling Listener pointer, re-entrant safe if callback mutates the map
	TFunction<void(const uint8*)> LocalCallback = MoveTemp(Listener->Callback);
	PendingRowListeners.Remove(Key);

	LocalCallback(RowData);
}

// Unsubscribes a specific row listener by (Owner, RowName), used for one-shot fire internally and optional explicit cleanup by consumers
void UDalRegistrySubsystem::UnbindFromDataRegistryRow(const UObject* Owner, FName RowName)
{
	if (!Owner
	    || RowName.IsNone())
	{
		return;
	}

	PendingRowListeners.Remove({const_cast<UObject*>(Owner), RowName});
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when engine subsystem initializes
void UDalRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

// Called when engine subsystem deinitializes, unbinds all owners
void UDalRegistrySubsystem::Deinitialize()
{
	// Collect all owners to unbind (iterate copy to avoid mutation during iteration)
	TArray<TWeakObjectPtr<const UObject>> AllOwners;
	OwnerBindings.GetKeys(AllOwners);
	for (const TWeakObjectPtr<const UObject>& WeakOwner : AllOwners)
	{
		UnbindFromDataRegistryLoad(WeakOwner.Get());
	}
	OwnerBindings.Empty();

	PendingRowListeners.Empty();

	Super::Deinitialize();
}

/*********************************************************************************************
 * Internal
 ********************************************************************************************* */

// Non-template implementation of BindAndLoad for a single struct
void UDalRegistrySubsystem::BindInternal(const UScriptStruct* InStruct, UObject* Owner, TDelegate<void()> OnChanged)
{
	FDalRegistryBinding& Binding = OwnerBindings.FindOrAdd(Owner);
	Binding.ChangedCallback = MoveTemp(OnChanged);

	AddAndLoad(Owner, InStruct);
}

// Non-template implementation of BindAndLoadFamily for a base struct
void UDalRegistrySubsystem::BindFamilyInternal(const UScriptStruct* InBaseStruct, UObject* Owner, TDelegate<void()> OnChanged)
{
	FDalRegistryBinding& Binding = OwnerBindings.FindOrAdd(Owner);
	Binding.ChangedCallback = MoveTemp(OnChanged);

	WaitForRegistries(Owner, [Owner, InBaseStruct]()
	{
		FDalRegistryRow::ForEachRegistry(InBaseStruct, [Owner](UDataRegistry* /*Registry*/, const UScriptStruct* ItemStruct)
		{
			Get().AddAndLoad(Owner, ItemStruct);
		});
	});
}

// Non-template implementation of Add
void UDalRegistrySubsystem::AddAndLoad(UObject* Owner, const UScriptStruct* InStruct)
{
	FDalRegistryBinding* Binding = OwnerBindings.Find(Owner);
	if (!ensureMsgf(InStruct, TEXT("ASSERT: [%i] %hs:\n'InStruct' is null!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(Binding, TEXT("ASSERT: [%i] %hs:\n'Owner' has no binding! Bind must be called before Add."), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (Binding->BoundStructs.Contains(InStruct))
	{
		if (!UDalUtilsLibrary::IsOwnerWorldStale(Owner))
		{
			Binding->ChangedCallback.ExecuteIfBound();
		}
		return;
	}

	Binding->BoundStructs.Add(InStruct);

	WaitForRegistries(Owner, [Owner, InStruct]()
	{
		UDalRegistrySubsystem& Subsystem = Get();
		Subsystem.BindOnRegistryChanged(InStruct, Owner, TDelegate<void(const UScriptStruct*)>::CreateLambda([WeakOwner = TWeakObjectPtr<const UObject>(Owner)](const UScriptStruct* ChangedStruct)
		{
			Get().TryLoad(WeakOwner.Get());
		}));

		Subsystem.TryLoad(Owner);
	});
}

// Runs Continuation immediately if Data Registry subsystem is initialized, otherwise defers it
bool UDalRegistrySubsystem::WaitForRegistries(const UObject* Owner, TFunction<void()> Continuation)
{
	UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	if (!ensureMsgf(DRSubsystem, TEXT("ASSERT: [%i] %hs:\nData Registry subsystem not found!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(Owner, TEXT("ASSERT: [%i] %hs:\n'Owner' is null! Can't wait for registries to initialize for binding."), __LINE__, __FUNCTION__))
	{
		return false;
	}

	if (DRSubsystem->AreRegistriesInitialized())
	{
		Continuation();
		return true;
	}

	TSharedRef<FDelegateHandle> DeferredHandle = MakeShared<FDelegateHandle>();
	*DeferredHandle = DRSubsystem->OnSubsystemInitialized().Add(FDataRegistrySubsystemInitializedCallback::FDelegate::CreateLambda([DeferredHandle, WeakOwner = TWeakObjectPtr(Owner), Continuation = MoveTemp(Continuation)]()
	{
		if (UDataRegistrySubsystem* DRSubsystemResolved = UDataRegistrySubsystem::Get())
		{
			DRSubsystemResolved->OnSubsystemInitialized().Remove(*DeferredHandle);
		}

		if (WeakOwner.IsValid())
		{
			Continuation();
		}
	}));
	return false;
}

// Binds delegate to the Data Registry cache change callback for InStruct, skips if Object is already bound
void UDalRegistrySubsystem::BindOnRegistryChanged(const UScriptStruct* InStruct, UObject* Object, TDelegate<void(const UScriptStruct*)> Delegate)
{
	if (!ensureMsgf(InStruct, TEXT("ASSERT: [%i] %hs:\n'InStruct' is null! Can't bind to registry change events."), __LINE__, __FUNCTION__)
	    || !ensureMsgf(Object, TEXT("ASSERT: [%i] %hs:\n'Object' is null! Can't bind to registry change events for struct '%s'."), __LINE__, __FUNCTION__, *InStruct->GetName()))
	{
		return;
	}

	// WaitForRegistries guarantees registries are initialized before this is called
	UDataRegistry* Registry = FDalRegistryRow::GetRegistryForStruct(InStruct);
	if (!ensureMsgf(Registry, TEXT("ASSERT: [%i] %hs:\nRegistry not found for struct '%s'! Struct is not registered in any Data Registry."), __LINE__, __FUNCTION__, *InStruct->GetName()))
	{
		return;
	}

	FDataRegistryCacheVersionCallback& CacheDelegate = Registry->OnCacheVersionInvalidated();
	if (CacheDelegate.IsBoundToObject(Object))
	{
		return;
	}

	CacheDelegate.Add(FDataRegistryCacheVersionCallback::FDelegate::CreateWeakLambda(Object, [InStruct, Delegate = MoveTemp(Delegate), WeakObject = TWeakObjectPtr<UObject>(Object)](UDataRegistry*)
	{
		if (!UDalUtilsLibrary::IsOwnerWorldStale(WeakObject.Get()))
		{
			Delegate.ExecuteIfBound(InStruct);
		}
	}));
}

// Removes all Object's bindings from the Data Registry cache change delegate for InStruct
void UDalRegistrySubsystem::UnbindOnRegistryChanged(const UScriptStruct* InStruct, const UObject* Object)
{
	UDataRegistry* Registry = FDalRegistryRow::GetRegistryForStruct(InStruct);
	if (!Registry)
	{
		return;
	}

	Registry->OnCacheVersionInvalidated().RemoveAll(Object);
}

// Returns true if every TSoftObjectPtr property on RowData is either null or has a loaded asset
bool UDalRegistrySubsystem::AreSoftRefsLoadedForRow(const UScriptStruct* InStruct, const uint8* RowData)
{
	if (!InStruct
	    || !RowData)
	{
		return false;
	}

	const TArray<const FSoftObjectProperty*>& SoftProps = UDalUtilsLibrary::GetSoftProperties(InStruct);
	for (const FSoftObjectProperty* SoftProp : SoftProps)
	{
		const FSoftObjectPtr* SoftPtr = SoftProp->ContainerPtrToValuePtr<FSoftObjectPtr>(RowData);
		if (SoftPtr
		    && !SoftPtr->IsNull()
		    && !SoftPtr->IsValid())
		{
			return false;
		}
	}
	return true;
}

// Auto-discovers all TSoftObjectPtr/TSoftClassPtr properties on InStruct via reflection
void UDalRegistrySubsystem::GatherAllSoftPaths(const UScriptStruct* InStruct, TArray<FSoftObjectPath>& OutPaths)
{
	const UDataRegistry* Registry = FDalRegistryRow::GetRegistryForStruct(InStruct);
	if (!Registry)
	{
		return;
	}

	const TArray<const FSoftObjectProperty*>& SoftProps = UDalUtilsLibrary::GetSoftProperties(InStruct);
	if (SoftProps.IsEmpty())
	{
		return;
	}

	TArray<FDataRegistryId> AllIds;
	Registry->GetPossibleRegistryIds(AllIds, /*bSortForDisplay=*/false);

	for (const FDataRegistryId& Id : AllIds)
	{
		// Bypass the engine cache: the source fills PrecachedDataPtr directly when bPrecacheTable=true, which is the project default
		FDataRegistryLookup Lookup;
		const uint8* RowData = nullptr;
		if (!Registry->ResolveDataRegistryId(Lookup, Id, &RowData)
		    || !RowData)
		{
			continue;
		}

		for (const FSoftObjectProperty* SoftProp : SoftProps)
		{
			const FSoftObjectPtr* SoftPtr = SoftProp->ContainerPtrToValuePtr<FSoftObjectPtr>(RowData);
			if (SoftPtr
			    && !SoftPtr->IsNull())
			{
				OutPaths.AddUnique(SoftPtr->ToSoftObjectPath());
			}
		}
	}
}
