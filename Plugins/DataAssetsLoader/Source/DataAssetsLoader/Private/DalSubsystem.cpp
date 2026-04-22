// Copyright (c) Yevhenii Selivanov

#include "DalSubsystem.h"

// DAL
#include "DalPrimaryDataAsset.h"
#include "DalUtilsLibrary.h"

// UE
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DalSubsystem)

// Returns this Subsystem, is checked and will crash if can't be obtained
UDalSubsystem& UDalSubsystem::Get()
{
	UDalSubsystem* Subsystem = GetDataAssetsLoaderSubsystem();
	checkf(Subsystem, TEXT("ASSERT: [%i] %hs:\n'Subsystem' is null"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this Subsystem, nullptr if engine is not available (e.g. during shutdown)
UDalSubsystem* UDalSubsystem::GetDataAssetsLoaderSubsystem()
{
	return GEngine ? GEngine->GetEngineSubsystem<UDalSubsystem>() : nullptr;
}

/*********************************************************************************************
 * Getters
 ********************************************************************************************* */

// Returns the data asset by its class, or nullptr if not registered
const UDalPrimaryDataAsset* UDalSubsystem::GetDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass)
{
	UDalSubsystem* Subsystem = GetDataAssetsLoaderSubsystem();
	const TWeakObjectPtr<const UDalPrimaryDataAsset>* FoundDataAsset = Subsystem ? Subsystem->DataAssetsMap.Find(*GetNameSafe(DataAssetClass)) : nullptr;
	return FoundDataAsset ? FoundDataAsset->Get() : nullptr;
}

/*********************************************************************************************
 * Listeners
 ********************************************************************************************* */

// Blueprint-only listener wrapped by K2Node_ListenForDataAsset
void UDalSubsystem::BPListenForDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, const FOnDalDataAssetLoaded& Completed)
{
	ListenForDataAssetInternal(Completed.GetUObject(), DataAssetClass, [Completed](const UDalPrimaryDataAsset& DataAsset)
	{
		Completed.ExecuteIfBound(&DataAsset);
	});
}

// Listens for multiple data asset classes at once, fires Completed callback when ALL are loaded
void UDalSubsystem::ListenForDataAssets(const UObject* Owner, const TArray<TSubclassOf<UDalPrimaryDataAsset>>& DataAssetClasses, TFunction<void()>&& Completed)
{
	if (DataAssetClasses.IsEmpty())
	{
		Completed();
		return;
	}

	const TSharedRef<int32> RemainingCount = MakeShared<int32>(DataAssetClasses.Num());
	for (const TSubclassOf<UDalPrimaryDataAsset>& DataAssetClass : DataAssetClasses)
	{
		ListenForDataAssetInternal(Owner, DataAssetClass, [RemainingCount, Completed](const UDalPrimaryDataAsset&)
		{
			--(*RemainingCount);
			if (*RemainingCount <= 0)
			{
				Completed();
			}
		});
	}
}

// Checks if data asset is already loaded and fires callback immediately, otherwise queues for later
void UDalSubsystem::ListenForDataAssetInternal(const UObject* Owner, TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, TFunction<void(const UDalPrimaryDataAsset&)>&& Callback)
{
	if (!ensureMsgf(Owner, TEXT("ASSERT: [%i] %hs:\n'Owner' is null, pass the subsystem itself for lifetime-untracked listeners!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (const UDalPrimaryDataAsset* DataAsset = GetDataAsset(DataAssetClass))
	{
		// Is already registered, fire immediately
		Callback(*DataAsset);
		return;
	}

	// Queue for later, will be fired in RegisterDataAsset
	const FName ClassName = *GetNameSafe(DataAssetClass);
	ensureMsgf(!ClassName.IsNone(), TEXT("ASSERT: [%i] %hs:\n'DataAssetClass' is not valid!"), __LINE__, __FUNCTION__);
	PendingListenersMap.FindOrAdd(ClassName).Emplace(FDalPendingDataAssetListener{Owner, MoveTemp(Callback)});
}

/*********************************************************************************************
 * Registration
 ********************************************************************************************* */

// Registers a data asset in the centralized cache and notifies pending per-class listeners
void UDalSubsystem::RegisterDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass, const UDalPrimaryDataAsset* DataAsset)
{
	if (!ensureMsgf(DataAssetClass, TEXT("ASSERT: [%i] %hs:\n'DataAssetClass' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(DataAsset, TEXT("ASSERT: [%i] %hs:\n'DataAsset' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const FName ClassName = DataAssetClass->GetFName();
	DataAssetsMap.Add(ClassName, DataAsset);

	// Notify and consume pending per-class listeners, silently drop those whose owner has died
	TArray<FDalPendingDataAssetListener> PendingListeners;
	if (PendingListenersMap.RemoveAndCopyValue(ClassName, PendingListeners))
	{
		for (const FDalPendingDataAssetListener& Listener : PendingListeners)
		{
			if (Listener.WeakOwner.IsValid())
			{
				Listener.Callback(*DataAsset);
			}
		}
	}
}

// Unregisters a data asset from the centralized cache
void UDalSubsystem::UnregisterDataAsset(TSubclassOf<UDalPrimaryDataAsset> DataAssetClass)
{
	if (!DataAssetClass)
	{
		return;
	}

	const FName ClassName = DataAssetClass->GetFName();
	DataAssetsMap.Remove(ClassName);
}

// Requests Asset Manager to load all primary data assets, each one triggers PostLoad -> RegisterDataAsset
void UDalSubsystem::TryLoadDataAssetsOnce(const UObject* OptionalWorldContext /* = nullptr*/)
{
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateLambda([WeakContext = TWeakObjectPtr(OptionalWorldContext)]
	{
		const UObject* InOptionalWorldContext = WeakContext.Get();
		if (!InOptionalWorldContext
		    || Get().LoadStreamableHandle)
		{
			// Already processed, skip
			return;
		}

		static const FPrimaryAssetType DataAssetType(UDalPrimaryDataAsset::StaticClass()->GetFName());
		Get().LoadStreamableHandle = UAssetManager::Get().LoadPrimaryAssetsWithType(DataAssetType, {}, {}, FStreamableManager::AsyncLoadHighPriority);
	}));
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Binds to world delegates and registers as game feature observer
void UDalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnBeginPlay);

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Unbinds from world delegates and removes game feature observer
void UDalSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	LoadStreamableHandle.Reset();

	Super::Deinitialize();
}

// Triggers async loading of primary data assets discovered under the game feature plugin path
void UDalSubsystem::OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	TArray<FPrimaryAssetId> AssetIds;
	UDalUtilsLibrary::GetAssetsInGameFeaturePlugin(/*out*/ AssetIds, GameFeatureData);
	if (!AssetIds.IsEmpty())
	{
		UAssetManager::Get().LoadPrimaryAssets(AssetIds, {}, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}
}

// Unloads primary data assets belonging to the game feature plugin, releasing streamable handles so GC can collect them
void UDalSubsystem::OnGameFeatureUnloading(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	TArray<FPrimaryAssetId> AssetIds;
	UDalUtilsLibrary::GetAssetsInGameFeaturePlugin(/*out*/ AssetIds, GameFeatureData);
	if (!AssetIds.IsEmpty())
	{
		UAssetManager::Get().UnloadPrimaryAssets(AssetIds);
	}
}

// Triggers loading of all discovered primary data assets when world begins play
void UDalSubsystem::OnBeginPlay(UWorld* World, FWorldInitializationValues WorldInitializationValues)
{
	TryLoadDataAssetsOnce(World);
}
