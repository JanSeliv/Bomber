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

#if WITH_EDITOR
#include "TimerManager.h"
#endif // WITH_EDITOR

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
 * Binding
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
void UDalRegistrySubsystem::Unbind(const UObject* Owner)
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
	bool bAllRegistriesAvailable = true;
	for (const UScriptStruct* StructIt : Binding->BoundStructs)
	{
		if (!FDalRegistryRow::GetRegistryForStruct(StructIt))
		{
			bAllRegistriesAvailable = false;
		}
		GatherAllSoftPaths(StructIt, PathsToLoad);
	}

	if (PathsToLoad.IsEmpty())
	{
		// Only mark loaded if all registries are available (empty means rows exist but has no soft assets to load)
		Binding->bIsLoaded = bAllRegistriesAvailable;
		if (!UDalUtilsLibrary::IsOwnerWorldStale(Owner))
		{
			Binding->ChangedCallback.ExecuteIfBound();
		}
		return;
	}

	// Use weak owner to protect binding access in async callback
	TWeakObjectPtr<const UObject> WeakOwner(Owner);
	Binding->StreamableHandle = RequestAsyncLoadPIESafe(Owner, MoveTemp(PathsToLoad), TDelegate<void()>::CreateLambda([WeakOwner]
	{
		UDalRegistrySubsystem* Subsystem = GetDalRegistrySubsystem();
		FDalRegistryBinding* AsyncBinding = Subsystem ? Subsystem->OwnerBindings.Find(WeakOwner) : nullptr;
		if (AsyncBinding)
		{
			AsyncBinding->bIsLoaded = true;
			if (!UDalUtilsLibrary::IsOwnerWorldStale(WeakOwner.Get()))
			{
				AsyncBinding->ChangedCallback.ExecuteIfBound();
			}
		}
	}));
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
		Unbind(WeakOwner.Get());
	}
	OwnerBindings.Empty();

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
	    || !ensureMsgf(Object, TEXT("ASSERT: [%i] %hs:\n'Object' is null! Can't bind to registry change events for struct '%s'."), __LINE__, __FUNCTION__, *InStruct->GetName())
	    || UDalUtilsLibrary::IsEditorNotPieWorld())
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

// Auto-discovers all TSoftObjectPtr/TSoftClassPtr properties on InStruct via reflection
void UDalRegistrySubsystem::GatherAllSoftPaths(const UScriptStruct* InStruct, TArray<FSoftObjectPath>& OutPaths)
{
	const UDataRegistry* Registry = FDalRegistryRow::GetRegistryForStruct(InStruct);
	if (!Registry)
	{
		return;
	}

	TMap<FDataRegistryId, const uint8*> CachedItems;
	const UScriptStruct* ItemStruct = nullptr;
	Registry->GetAllCachedItems(CachedItems, ItemStruct);

	const TArray<const FSoftObjectProperty*>& SoftProps = UDalUtilsLibrary::GetSoftProperties(InStruct);

	for (const FSoftObjectProperty* SoftProp : SoftProps)
	{
		for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
		{
			const FSoftObjectPtr* SoftPtr = SoftProp->ContainerPtrToValuePtr<FSoftObjectPtr>(Pair.Value);
			if (SoftPtr && !SoftPtr->IsNull())
			{
				OutPaths.AddUnique(SoftPtr->ToSoftObjectPath());
			}
		}
	}
}

// PIE-safe async load request
TSharedPtr<FStreamableHandle> UDalRegistrySubsystem::RequestAsyncLoadPIESafe(const UObject* WorldContextObject, TArray<FSoftObjectPath> PathsToLoad, TDelegate<void()> OnComplete)
{
	if (PathsToLoad.IsEmpty())
	{
		return nullptr;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

#if WITH_EDITOR
	// In editor, defer callback to next tick to avoid cross-world interference in PIE multiplayer
	if (UDalUtilsLibrary::IsEditor())
	{
		const TWeakObjectPtr WeakContext(WorldContextObject);
		TSharedRef<TDelegate<void()>> SharedCallback = MakeShared<TDelegate<void()>>(MoveTemp(OnComplete));

		return StreamableManager.RequestAsyncLoad(MoveTemp(PathsToLoad), FStreamableDelegate::CreateLambda([WeakContext, SharedCallback]()
		{
			if (const UWorld* World = UDalUtilsLibrary::GetPlayWorld(WeakContext.Get()))
			{
				World->GetTimerManager().SetTimerForNextTick([WeakContext, SharedCallback]()
				{
					if (WeakContext.IsValid())
					{
						SharedCallback->ExecuteIfBound();
					}
				});
			}
		}));
	}
#endif // WITH_EDITOR

	return StreamableManager.RequestAsyncLoad(MoveTemp(PathsToLoad), FStreamableDelegate::CreateLambda([OnComplete = MoveTemp(OnComplete)]()
	{
		OnComplete.ExecuteIfBound();
	}));
}
