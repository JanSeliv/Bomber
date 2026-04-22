// Copyright (c) Yevhenii Selivanov

#include "DalUtilsLibrary.h"

// DAL
#include "DalPrimaryDataAsset.h"
#include "DalRegistryRow.h"

// UE
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeatureData.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(DalUtilsLibrary)

// Returns the first found asset data for a UDalPrimaryDataAsset descendant matching a single registry tag
FAssetData UDalUtilsLibrary::GetAssetByRegistryTag(FName AssetTag, const FString& AssetValue)
{
	if (AssetTag.IsNone()
	    || AssetValue.IsEmpty())
	{
		return FAssetData();
	}

	TMultiMap<FName, TOptional<FString>> TagsAndValues;
	TagsAndValues.Add(AssetTag, AssetValue);

	TArray<FAssetData> AssetsData;
	GetAssetsByRegistryTags(/*out*/ AssetsData, TagsAndValues);

	return AssetsData.IsValidIndex(0) ? AssetsData[0] : FAssetData();
}

// Returns asset data for all registered UDalPrimaryDataAsset descendants matching the specified registry tags
void UDalUtilsLibrary::GetAssetsByRegistryTags(TArray<FAssetData>& OutAssetsData, const TMultiMap<FName, TOptional<FString>>& TagsAndValues)
{
	if (TagsAndValues.IsEmpty())
	{
		return;
	}

	FARFilter Filter;
	static const FTopLevelAssetPath AssetClassPath = UDalPrimaryDataAsset::StaticClass()->GetClassPathName();
	Filter.ClassPaths.Add(AssetClassPath);
	Filter.bRecursiveClasses = true;
	Filter.TagsAndValues = TagsAndValues;
	UAssetManager::Get().GetAssetRegistry().GetAssets(Filter, OutAssetsData);
}

// Returns primary asset IDs for all UDalPrimaryDataAsset descendants discovered under the game feature plugin root path
void UDalUtilsLibrary::GetAssetsInGameFeaturePlugin(TArray<FPrimaryAssetId>& OutAssetIds, const UGameFeatureData* GameFeatureData)
{
	if (!GameFeatureData)
	{
		return;
	}

	FString PluginRootPath;
	FString OutPackagePath;
	FString OutPackageName;
	FPackageName::SplitLongPackageName(GameFeatureData->GetPathName(), PluginRootPath, OutPackagePath, OutPackageName);
	PluginRootPath.RemoveFromEnd(TEXT("/"));
	if (PluginRootPath.IsEmpty())
	{
		return;
	}

	FARFilter Filter;
	static const FTopLevelAssetPath AssetClassPath = UDalPrimaryDataAsset::StaticClass()->GetClassPathName();
	Filter.ClassPaths.Add(AssetClassPath);
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add(*PluginRootPath);
	Filter.bRecursivePaths = true;

	TArray<FAssetData> FoundAssets;
	UAssetManager::Get().GetAssetRegistry().GetAssets(Filter, FoundAssets);
	for (const FAssetData& AssetData : FoundAssets)
	{
		OutAssetIds.Emplace(AssetData.GetPrimaryAssetId());
	}
}

/*********************************************************************************************
 * Data Registry
 ********************************************************************************************* */

// Returns all cached rows from Data Registry as a typed array matching InRowStruct
void UDalUtilsLibrary::K2_GetAllRegistryRowsGeneric(const UScriptStruct* InRowStruct, const FArrayProperty* ArrayProp, void* OutArrayPtr)
{
	const FStructProperty* InnerProp = ArrayProp ? CastField<FStructProperty>(ArrayProp->Inner) : nullptr;
	if (!InRowStruct || !InnerProp || !OutArrayPtr)
	{
		return;
	}

	const UScriptStruct* ElementStruct = InnerProp->Struct;
	if (ElementStruct != InRowStruct)
	{
		return;
	}

	FScriptArrayHelper ArrayHelper(ArrayProp, OutArrayPtr);
	ArrayHelper.EmptyValues();

	FDalRegistryRow::ForEachRow(InRowStruct, [&ArrayHelper, ElementStruct](const uint8* ItemData)
	{
		const int32 NewIndex = ArrayHelper.AddValue();
		uint8* DestPtr = ArrayHelper.GetElementPtr(NewIndex);
		ElementStruct->CopyScriptStruct(DestPtr, ItemData);
	});
}

DEFINE_FUNCTION(UDalUtilsLibrary::execK2_GetAllRegistryRows)
{
	P_GET_OBJECT(UScriptStruct, InRowStruct);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* OutArrayPtr = Stack.MostRecentPropertyAddress;
	const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_FINISH;

	P_NATIVE_BEGIN;
	K2_GetAllRegistryRowsGeneric(InRowStruct, ArrayProp, OutArrayPtr);
	P_NATIVE_END;
}

// Finds the Data Registry whose ItemStruct matches InStruct, cached for fast repeated access
UDataRegistry* UDalUtilsLibrary::GetRegistryForStruct(const UScriptStruct* InStruct)
{
	return FDalRegistryRow::GetRegistryForStruct(InStruct);
}

// Returns overall number of cached rows for the given row struct type
int32 UDalUtilsLibrary::GetRegistryRowsNum(const UScriptStruct* InStruct)
{
	return FDalRegistryRow::GetRowsNum(InStruct);
}

// Returns the row name at specified index for the given struct type, or NAME_None
FName UDalUtilsLibrary::GetRegistryRowNameByIndex(const UScriptStruct* InStruct, int32 Index)
{
	return FDalRegistryRow::GetRowNameByIndex(InStruct, Index);
}

/*********************************************************************************************
 * Internal Helpers
 ********************************************************************************************* */

// Executes callback with PIE-safe dispatch: defers in editor with weak context safety, direct call otherwise
void UDalUtilsLibrary::ExecutePIESafe(const UObject* ContextObject, TFunction<void()> Callback)
{
	if (!Callback
	    || !ContextObject)
	{
		return;
	}

#if WITH_EDITOR
	if (GIsEditor)
	{
		const UWorld* World = GetPlayWorld(ContextObject);
		const bool bWorldUsable = World && !World->bIsTearingDown && !IsGarbageCollecting();
		if (!bWorldUsable)
		{
			return;
		}

		World->GetTimerManager().SetTimerForNextTick([WeakContext = TWeakObjectPtr(ContextObject), Callback = MoveTemp(Callback)]()
		{
			if (WeakContext.IsValid())
			{
				Callback();
			}
		});
		return;
	}
#endif // WITH_EDITOR

	Callback();
}

// Returns true if the owner's world is in a transitional state where callbacks should be suppressed
bool UDalUtilsLibrary::IsOwnerWorldStale(const UObject* Owner)
{
	if (!GEngine)
	{
		return true;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(Owner, EGetWorldErrorMode::ReturnNull);
	if (!World || World->bIsTearingDown)
	{
		return true;
	}

	// Network transition: Browse reconfigured net driver (NetMode shifted to Client)
	// but TickWorldTravel has not yet called LoadMap to create proper client world.
	// Cooked builds hit this race because pak-cached assets load faster than engine ticks
	const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World);
	if (WorldContext && WorldContext->PendingNetGame)
	{
		return true;
	}

#if WITH_EDITOR
	if (GEditor && GEditor->ShouldEndPlayMap())
	{
		return true;
	}
#endif

	return false;
}

// Returns the current play world as UObject for weak pointer storage
UWorld* UDalUtilsLibrary::GetPlayWorld(const UObject* WorldContextObject)
{
	UWorld* FoundWorld = nullptr;
	if (GEngine)
	{
		FoundWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (!FoundWorld)
		{
			FoundWorld = GEngine->GetCurrentPlayWorld();
		}
	}

#if WITH_EDITOR
	if (!FoundWorld && GEditor)
	{
		FoundWorld = GEditor->GetEditorWorldContext().World();
	}
#endif

	if (!FoundWorld)
	{
		FoundWorld = GWorld;
	}

	return FoundWorld;
}

// Cached soft property lists per struct type for GatherAllSoftPaths
TMap<const UScriptStruct*, TArray<const FSoftObjectProperty*>>& UDalUtilsLibrary::GetSoftPropsCache()
{
	static TMap<const UScriptStruct*, TArray<const FSoftObjectProperty*>> Cache;
	return Cache;
}

// Returns cached array of soft object/class properties for the given struct
const TArray<const FSoftObjectProperty*>& UDalUtilsLibrary::GetSoftProperties(const UScriptStruct* InStruct)
{
	TMap<const UScriptStruct*, TArray<const FSoftObjectProperty*>>& Cache = GetSoftPropsCache();
	if (const TArray<const FSoftObjectProperty*>* Found = Cache.Find(InStruct))
	{
		return *Found;
	}

	TArray<const FSoftObjectProperty*>& Props = Cache.Add(InStruct);
	for (TFieldIterator<FSoftObjectProperty> PropIt(InStruct, EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
	{
		Props.Add(*PropIt);
	}
	return Props;
}

// Returns true if currently in editor mode (not packaged, not -game)
bool UDalUtilsLibrary::IsEditor()
{
#if WITH_EDITOR
	return GIsEditor && GEditor && GWorld && GWorld->IsEditorWorld();
#else
	return false;
#endif
}
