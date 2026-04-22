// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"

// UE
#include "GameFeatureStateChangeObserver.h"
#include "Templates/SubclassOf.h"

#include "DalSubsystem.generated.h"

class UDalPrimaryDataAsset;

/**
 * Centralized engine subsystem that caches all UDalPrimaryDataAsset data assets loaded by Asset Registry and registered in UDalPrimaryDataAsset::PostLoad
 */
UCLASS(BlueprintType, Blueprintable)
class DATAASSETSLOADER_API UDalSubsystem : public UEngineSubsystem,
                                           public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained */
	static UDalSubsystem& Get();

	/** Returns the pointer to this Subsystem, nullptr if engine is not available (e.g. during shutdown) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	static UDalSubsystem* GetDataAssetsLoaderSubsystem();

	/*********************************************************************************************
	 * Getters
	 ********************************************************************************************* */
public:
	/** Returns the data asset by its class, or nullptr if not registered */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]", meta = (DeterminesOutputType = "DataAssetClass"))
	static const UDalPrimaryDataAsset* GetDataAsset(
	    UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UDalPrimaryDataAsset> DataAssetClass);

	/** Returns the data asset by its class, or nullptr if subsystem or data asset is not available.
	 * Example: `const UMyGameDataAsset* DA = UDalSubsystem::GetDataAsset<UMyGameDataAsset>(); */
	template <typename T = UDalPrimaryDataAsset>
	static const FORCEINLINE T* GetDataAsset() { return Cast<T>(GetDataAsset(T::StaticClass())); }

	/** Returns casted data asset by its class, or nullptr if not registered
	 * Example: `const UMyGameDataAsset* DA = UDalSubsystem::GetDataAsset<UMyGameDataAsset>(Property);` */
	template <typename T = UDalPrimaryDataAsset>
	static const FORCEINLINE T* GetDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass) { return Cast<T>(GetDataAsset(DataAssetClass)); }

	/** Returns the data asset by its class, crashes if not found.
	 * Example: `const UMyGameDataAsset& DA = UDalSubsystem::GetDataAssetChecked<UMyGameDataAsset>();` */
	template <typename T = UDalPrimaryDataAsset>
	static const FORCEINLINE T& GetDataAssetChecked() { return *CastChecked<T>(GetDataAsset(T::StaticClass())); }

	/*********************************************************************************************
	 * Listeners
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDalDataAssetLoaded, const UDalPrimaryDataAsset*, DataAsset);

	/** Listens for a data asset of the specified class, fires Completed when loaded or immediately if already available.
	 * Blueprint-only listener node, in code use the templated ListenForDataAsset() instead */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]", DisplayName = "Listen For Data Asset [DAL]", meta = (BlueprintInternalUseOnly = "true"))
	void BPListenForDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, const FOnDalDataAssetLoaded& Completed);

	/** Runtime class with member function and weak object safety.
	 * Example: ListenForDataAsset(SomeClass, this, &ThisClass::OnDataAssetLoaded); where function is `void OnDataAssetLoaded(const UDalPrimaryDataAsset* DA)` */
	template <typename TOwner>
	void ListenForDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, TOwner* Object, void (TOwner::*Function)(const UDalPrimaryDataAsset*));

	/** Runtime class with typed lambda callback and weak object safety.
	 * Example: ListenForDataAsset<UMyGameDataAsset>(this, SomeClass, [](const UMyGameDataAsset& DA) { ... }); */
	template <typename T>
	void ListenForDataAsset(const UObject* Owner, TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, TFunction<void(const T&)>&& Callback);

	/** Compile-time class with typed lambda callback and weak object safety.
	 * Example: ListenForDataAsset<UMyGameDataAsset>(this, [](const UMyGameDataAsset& DA) { DA.Init(); }); */
	template <typename T>
	void ListenForDataAsset(const UObject* Owner, TFunction<void(const T&)>&& Callback);

	/** Compile-time class with typed member function and weak object safety.
	 * Example: ListenForDataAsset<UMyGameDataAsset>(this, &ThisClass::OnLoaded); where function is `void OnLoaded(const UMyGameDataAsset* DA)` */
	template <typename T, typename TOwner = UObject>
	void ListenForDataAsset(TOwner* Object, void (TOwner::*Function)(const T*));

	/** Listens for multiple data asset classes at once, fires Completed callback when ALL are loaded, with weak object safety.
	 * Already loaded data assets are counted immediately, remaining ones are queued for listening. */
	void ListenForDataAssets(const UObject* Owner, const TArray<TSubclassOf<UDalPrimaryDataAsset>>& DataAssetClasses, TFunction<void()>&& Completed);

protected:
	/** Checks if data asset is already loaded and fires callback immediately, otherwise queues for later.
	 * Guarantees the callback never fires after Owner has been destroyed. */
	void ListenForDataAssetInternal(const UObject* Owner, TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, TFunction<void(const UDalPrimaryDataAsset&)>&& Callback);

	/*********************************************************************************************
	 * Registration
	 ********************************************************************************************* */
public:
	/** Registers a data asset in the centralized cache and notifies pending per-class listeners */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]")
	void RegisterDataAsset(
	    UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UDalPrimaryDataAsset> DataAssetClass,
	    const UDalPrimaryDataAsset* DataAsset);

	/** Unregisters a data asset from the centralized cache */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]")
	void UnregisterDataAsset(
	    UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UDalPrimaryDataAsset> DataAssetClass);

protected:
	/** Requests Asset Manager to load all primary data assets, each one triggers PostLoad -> RegisterDataAsset */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]", meta = (BlueprintProtected, WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	void TryLoadDataAssetsOnce(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Binds to world delegates and registers as game feature observer */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Unbinds from world delegates and removes game feature observer */
	virtual void Deinitialize() override;

	/** Triggers async loading of primary data assets discovered under the game feature plugin path */
	virtual void OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/** Unloads primary data assets belonging to the game feature plugin, releasing streamable handles so GC can collect them */
	virtual void OnGameFeatureUnloading(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/** Triggers loading of all discovered primary data assets when world begins play */
	void OnBeginPlay(UWorld* World, struct FWorldInitializationValues WorldInitializationValues);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Cached data assets keyed by class name, weak references allow garbage collection when modular features unload */
	TMap<FName /*ClassName*/, TWeakObjectPtr<const UDalPrimaryDataAsset>> DataAssetsMap;

	/** Tracks the async load request, used to prevent duplicate loads */
	TSharedPtr<struct FStreamableHandle> LoadStreamableHandle = nullptr;

	/** Passive listener entry, fires exactly once when its observed data asset class is registered */
	struct FDalPendingDataAssetListener
	{
		/** Weak owner, listener is silently dropped if invalid at fire time */
		TWeakObjectPtr<const UObject> WeakOwner = nullptr;

		/** Erased callback fired with the registered data asset */
		TFunction<void(const UDalPrimaryDataAsset&)> Callback = nullptr;
	};

	/** All pending passive listeners waiting for their data asset class to be registered */
	TMap<FName /*ClassName*/, TArray<FDalPendingDataAssetListener>> PendingListenersMap;
};

// Subscribes to a specific data asset class by runtime class via member function, lifetime guaranteed by Internal
template <typename TOwner>
void UDalSubsystem::ListenForDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, TOwner* Object, void (TOwner::*Function)(const UDalPrimaryDataAsset*))
{
	ListenForDataAssetInternal(Object, DataAssetClass, [Object, Function](const UDalPrimaryDataAsset& DataAsset)
	{
		(Object->*Function)(&DataAsset);
	});
}

// Runtime class with typed lambda callback, lifetime guaranteed by Internal
template <typename T>
void UDalSubsystem::ListenForDataAsset(const UObject* Owner, TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, TFunction<void(const T&)>&& Callback)
{
	static_assert(TIsDerivedFrom<T, UDalPrimaryDataAsset>::IsDerived, "T must derive from UDalPrimaryDataAsset");
	ListenForDataAssetInternal(Owner, DataAssetClass, [Callback = MoveTemp(Callback)](const UDalPrimaryDataAsset& DataAsset)
	{
		Callback(static_cast<const T&>(DataAsset));
	});
}

// Compile-time class with typed lambda callback, lifetime guaranteed by Internal
template <typename T>
void UDalSubsystem::ListenForDataAsset(const UObject* Owner, TFunction<void(const T&)>&& Callback)
{
	static_assert(TIsDerivedFrom<T, UDalPrimaryDataAsset>::IsDerived, "T must derive from UDalPrimaryDataAsset");
	ListenForDataAssetInternal(Owner, T::StaticClass(), [Callback = MoveTemp(Callback)](const UDalPrimaryDataAsset& DataAsset)
	{
		Callback(static_cast<const T&>(DataAsset));
	});
}

// Subscribes to a specific data asset class via member function, lifetime guaranteed by Internal
template <typename T, typename TOwner>
void UDalSubsystem::ListenForDataAsset(TOwner* Object, void (TOwner::*Function)(const T*))
{
	static_assert(TIsDerivedFrom<T, UDalPrimaryDataAsset>::IsDerived, "T must derive from UDalPrimaryDataAsset");
	ListenForDataAssetInternal(Object, T::StaticClass(), [Object, Function](const UDalPrimaryDataAsset& DataAsset)
	{
		(Object->*Function)(Cast<T>(&DataAsset));
	});
}
