// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"

#include "DalRegistrySubsystem.generated.h"

/**
 * Centralized engine subsystem that manages Data Registry consumer lifecycle:
 * subscribing to cache changes, async loading soft references, and cleanup on teardown.
 * Consumers call Bind/Unbind by their UObject* owner.
 */
UCLASS(BlueprintType, Blueprintable)
class DATAASSETSLOADER_API UDalRegistrySubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained */
	static UDalRegistrySubsystem& Get();

	/** Returns the pointer to this Subsystem, nullptr if engine is not available (e.g. during shutdown) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	static UDalRegistrySubsystem* GetDalRegistrySubsystem();

	/*********************************************************************************************
	 * Load
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_DELEGATE(FOnDalRegistryRowsChanged);

	/** Subscribes to a single Data Registry for cache change events and async loads its soft references.
	 * When cache changes, re-gathers all soft paths and fires OnChanged after async load completes.
	 * Example: `DalRegistry.BindAndLoad<FMyRow>(this, &ThisClass::OnRowsLoaded);` */
	template <typename TRow, typename UserClass>
	void BindAndLoad(UserClass* Owner, void (UserClass::*OnChanged)()) { BindInternal(TRow::StaticStruct(), Owner, TDelegate<void()>::CreateUObject(Owner, OnChanged)); }

	/** Subscribes to all Data Registries whose ItemStruct inherits TBaseRow.
	 * Discovers child registries at bind time via FDalRegistryRow::ForEachRegistry.
	 * All sources share a single batched async load and callback.
	 * Example: `DalRegistry.BindAndLoadFamily<FMyBaseRow>(this, &ThisClass::OnFamilyLoaded);` */
	template <typename TBaseRow, typename UserClass>
	void BindAndLoadFamily(UserClass* Owner, void (UserClass::*OnChanged)()) { BindFamilyInternal(TBaseRow::StaticStruct(), Owner, TDelegate<void()>::CreateUObject(Owner, OnChanged)); }

	/** Adds an extra row struct source to this owner's monitoring set.
	 * Must be called after Bind or BindAndLoadFamily. Shares the same StreamableHandle and callback.
	 * Example: `DalRegistry.Add<FMyExtraRow>(this);` */
	template <typename TRow = UScriptStruct>
	void Add(UObject* Owner, const UScriptStruct* InStruct = TRow::StaticStruct()) { AddAndLoad(Owner, InStruct); }

	/** Blueprint-callable wrapper for BindAndLoad.
	 * Subscribes to a Data Registry by row struct type and fires Completed when rows change and soft references finish loading */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]", DisplayName = "Bind And Load Registry Rows [DAL]", meta = (BlueprintInternalUseOnly = "true"))
	void BPBindAndLoad(UObject* Owner, const UScriptStruct* RowStruct, const FOnDalRegistryRowsChanged& OnChanged);

	/** Unsubscribes the given owner from all bound Data Registries, cancels pending async load, resets binding */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]")
	void UnbindFromDataRegistryLoad(const UObject* Owner);

	/** Returns true if the given owner is currently bound to at least one Data Registry */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	bool IsBound(const UObject* Owner) const;

	/** Returns true if all soft references for the given owner have finished async loading at least once.
	 * When bound with no soft paths, considered loaded since callback already fired immediately */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	bool IsLoaded(const UObject* Owner) const;

	/** Re-gathers all soft paths for the given owner from all bound sources and starts async load, fires OnChanged on completion */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]")
	void TryLoad(const UObject* Owner);

	/*********************************************************************************************
	 * Listeners
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDalRegistryRowLoaded, FName, RowName);

	/** Pure one-shot observer: fires Completed once the specified row and its soft references are resolvable in the Data Registry.
	 * Does NOT trigger async loading itself, but relies on a separate BindAndLoad / BindAndLoadFamily call for the same struct (anywhere in the project) to drive async loading.
	 * Automatically unsubscribes after firing, Owner weak-lifetime safe.
	 * Blueprint-only, in code use the templated ListenForDataRegistryRow() instead */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]", DisplayName = "Listen For Data Registry Row [DAL]", meta = (BlueprintInternalUseOnly = "true"))
	void BPListenForDataRegistryRow(UObject* Owner, const FDataRegistryId& ItemId, const FOnDalRegistryRowLoaded& Completed);

	/** Runtime class with typed member function and weak object safety.
	 * Example: ListenForDataRegistryRow<FMyRow>(this, RowName, &ThisClass::OnRowLoaded); where function is `void OnRowLoaded(const FMyRow& Row)` */
	template <typename TRow, typename TOwner>
	void ListenForDataRegistryRow(TOwner* Object, FName RowName, void (TOwner::*Function)(const TRow&));

	/** Runtime class with typed lambda callback and weak object safety.
	 * Example: ListenForDataRegistryRow<FMyRow>(this, RowName, [this](const FMyRow& Row) { ... }); */
	template <typename TRow>
	void ListenForDataRegistryRow(UObject* Object, FName RowName, TFunction<void(const TRow&)>&& Callback);

	/** Runtime-struct lambda variant for polymorphic bases where the actual derived row type is only known at runtime, with weak object safety.
	 * Fires Callback(RowName) once the row and its soft references are resolvable, consumer re-resolves the row itself.
	 * Example: ListenForDataRegistryRow(this, GetRowType(), RowName, [this](FName RowName){ ... }); */
	void ListenForDataRegistryRow(UObject* Object, const UScriptStruct* InStruct, FName RowName, TFunction<void(FName)>&& Callback);

	/** Unsubscribes a specific row listener by (Owner, RowName), used for one-shot fire internally and optional explicit cleanup by consumers */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]")
	void UnbindFromDataRegistryRow(const UObject* Owner, FName RowName);

protected:
	/** Non-template implementation: queues a one-shot listener for the given row, fires immediately if the row is already available.
	 * Guarantees the callback never fires after Owner has been destroyed, public overloads rely on this contract and skip their own weak wrapping. */
	void ListenForDataRegistryRowInternal(UObject* Owner, const UScriptStruct* InStruct, FName RowName, TFunction<void(const uint8*)>&& Callback);

	/** Notifies all pending row listeners for InStruct, fires and removes those whose rows have become resolvable. */
	void NotifyPendingRowListeners(const UScriptStruct* InStruct);

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when engine subsystem initializes */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when engine subsystem deinitializes, unbinds all owners */
	virtual void Deinitialize() override;

	/*********************************************************************************************
	 * Internal
	 ********************************************************************************************* */
private:
	/** Per-owner binding data tracking all registry subscriptions and async load state */
	struct FDalRegistryBinding
	{
		/** All row struct types this binding is monitoring */
		TArray<const UScriptStruct*> BoundStructs;

		/** Callback fired after old rows are dropped and new rows' soft references finish async loading */
		TDelegate<void()> ChangedCallback;

		/** Keeps loaded asset packages in memory while the binding is alive */
		TSharedPtr<struct FStreamableHandle> StreamableHandle = nullptr;

		/** Set to true after the first successful async load completes */
		bool bIsLoaded = false;
	};

	/** All active bindings keyed by owner */
	TMap<TWeakObjectPtr<const UObject>, FDalRegistryBinding> OwnerBindings;

	/** Identifies a pending listener by its owner and the specific row it is waiting on */
	struct FDalRegistryRowListenerKey
	{
		/** Weak owner, listener is silently dropped if invalid at fire time */
		TWeakObjectPtr<UObject> WeakOwner = nullptr;

		/** Specific registry row name this listener is waiting on */
		FName RowName = NAME_None;

		bool operator==(const FDalRegistryRowListenerKey& Other) const { return WeakOwner == Other.WeakOwner && RowName == Other.RowName; }
		friend uint32 GetTypeHash(const FDalRegistryRowListenerKey& Key) { return HashCombine(GetTypeHash(Key.WeakOwner), GetTypeHash(Key.RowName)); }
	};

	/** Passive listener entry, fires exactly once when its observed row becomes resolvable */
	struct FDalRegistryRowListener
	{
		/** Struct this listener is observing, used to match notifications driven by a BindAndLoad for the same struct */
		TWeakObjectPtr<const UScriptStruct> WeakStruct;

		/** Erased callback, parameter is the raw row data of the bound struct */
		TFunction<void(const uint8*)> Callback = nullptr;
	};

	/** All pending passive listeners waiting for their rows to become resolvable */
	TMap<FDalRegistryRowListenerKey, FDalRegistryRowListener> PendingRowListeners;

	/** Fires and removes the identified listener if its row is now resolvable.  */
	void TryFireAndRemoveListener(const FDalRegistryRowListenerKey& Key, const UScriptStruct* InStruct);

	/** Non-template implementation of BindAndLoad for a single struct */
	void BindInternal(const UScriptStruct* InStruct, UObject* Owner, TDelegate<void()> OnChanged);

	/** Non-template implementation of BindAndLoadFamily for a base struct */
	void BindFamilyInternal(const UScriptStruct* InBaseStruct, UObject* Owner, TDelegate<void()> OnChanged);

	/** Non-template implementation of Add */
	void AddAndLoad(UObject* Owner, const UScriptStruct* InStruct);

	/** Runs Continuation immediately if Data Registry subsystem is initialized,
	 * otherwise defers it via OnSubsystemInitialized bound to Owner's weak lifetime.
	 * Single gate for all code paths that require registries to be available */
	static bool WaitForRegistries(const UObject* Owner, TFunction<void()> Continuation);

	/** Binds delegate to the Data Registry cache change callback for InStruct, skips if Object is already bound.
	 * Must be called after registries are initialized (caller gates via WaitForRegistries) */
	void BindOnRegistryChanged(const UScriptStruct* InStruct, UObject* Object, TDelegate<void(const UScriptStruct*)> Delegate);

	/** Removes all Object's bindings from the Data Registry cache change delegate for InStruct */
	static void UnbindOnRegistryChanged(const UScriptStruct* InStruct, const UObject* Object);

	/** Auto-discovers all TSoftObjectPtr/TSoftClassPtr properties on InStruct via reflection
	 * and gathers non-null soft paths from every cached row into OutPaths */
	static void GatherAllSoftPaths(const UScriptStruct* InStruct, TArray<FSoftObjectPath>& OutPaths);

	/** Returns true if every TSoftObjectPtr property on RowData is either null or has a loaded asset.
	 * Used to gate row-listener callbacks so consumers never observe a row with unresolved soft refs */
	static bool AreSoftRefsLoadedForRow(const UScriptStruct* InStruct, const uint8* RowData);
};

// Runtime class with typed member function, lifetime guaranteed by Internal
template <typename TRow, typename TOwner>
void UDalRegistrySubsystem::ListenForDataRegistryRow(TOwner* Object, FName RowName, void (TOwner::*Function)(const TRow&))
{
	ListenForDataRegistryRowInternal(Object, TRow::StaticStruct(), RowName, [Object, Function](const uint8* RowData)
	{
		(Object->*Function)(*reinterpret_cast<const TRow*>(RowData));
	});
}

// Runtime class with typed lambda callback, lifetime guaranteed by Internal
template <typename TRow>
void UDalRegistrySubsystem::ListenForDataRegistryRow(UObject* Object, FName RowName, TFunction<void(const TRow&)>&& Callback)
{
	ListenForDataRegistryRowInternal(Object, TRow::StaticStruct(), RowName, [Callback = MoveTemp(Callback)](const uint8* RowData)
	{
		Callback(*reinterpret_cast<const TRow*>(RowData));
	});
}
