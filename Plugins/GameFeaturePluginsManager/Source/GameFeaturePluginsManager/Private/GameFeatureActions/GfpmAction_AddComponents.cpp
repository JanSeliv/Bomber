// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_AddComponents.h"

// GFPM
#include "Data/GfpmScopedWorldContext.h"

// UE
#include "AssetRegistry/AssetBundleData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFeaturesSubsystemSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAction_AddComponents)

// When owning Game Feature Plugin transitions into Active state
void UGfpmAction_AddComponents::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FGfpmContextHandles& Handles = ContextHandles.FindOrAdd(Context);

	Handles.GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this,
	    &UGfpmAction_AddComponents::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));

	if (!ensureMsgf(Handles.ComponentRequestHandlesByWorld.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'Handles.ComponentRequestHandlesByWorld' is not empty!"), __LINE__, __FUNCTION__))
	{
		// Stale handles from prior activation without matching deactivation, releasing now to avoid double-populating
		Handles.ComponentRequestHandlesByWorld.Empty();
	}

	if (!GEngine)
	{
		// Engine not initialized, no world contexts to iterate
		return;
	}
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (!Context.ShouldApplyToWorldContext(WorldContext))
		{
			// World filtered out by activation context rules
			continue;
		}
		// Pin GWorld/PIE-ID to this world for synchronous create-on-existing-receivers cascade AddToWorld -> GFCM->AddComponentRequest triggers
		FGfpmScopedWorldContext WorldContextGuard(WorldContext.World());
		AddToWorld(WorldContext, Handles);
	}
}

// When owning Game Feature Plugin transitions out of Active state
void UGfpmAction_AddComponents::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FGfpmContextHandles* Handles = ContextHandles.Find(Context);
	if (!ensureMsgf(Handles, TEXT("ASSERT: [%i] %hs:\n'Handles for given Context' is null, deactivating context never went through activation"), __LINE__, __FUNCTION__))
	{
		return;
	}

	FWorldDelegates::OnStartGameInstance.Remove(Handles->GameInstanceStartHandle);
	Handles->GameInstanceStartHandle.Reset();

	if (GEngine)
	{
		// Pin GWorld/PIE-ID per bucket so each handle destructor (RemoveComponentRequest -> component OnUnregister) lands in correct world's pinned context, mirror of activation scope guard
		for (TPair<FName, TArray<TSharedPtr<FComponentRequestHandle>>>& Bucket : Handles->ComponentRequestHandlesByWorld)
		{
			UWorld* BucketWorld = nullptr;
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				if (WorldContext.ContextHandle == Bucket.Key)
				{
					BucketWorld = WorldContext.World();
					break;
				}
			}
			FGfpmScopedWorldContext WorldContextGuard(BucketWorld);
			// Releasing handles removes components from registered actors in this world
			Bucket.Value.Empty();
		}
	}
	Handles->ComponentRequestHandlesByWorld.Empty();
	ContextHandles.Remove(Context);
}

#if WITH_EDITORONLY_DATA
// When cooker gathers asset bundle data for owning plugin
void UGfpmAction_AddComponents::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsInitialized())
	{
		for (const FGameFeatureComponentEntry& Entry : ComponentList)
		{
			if (Entry.bClientComponent)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.ComponentClass.ToSoftObjectPath().GetAssetPath());
			}
			if (Entry.bServerComponent)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, Entry.ComponentClass.ToSoftObjectPath().GetAssetPath());
			}
		}
	}
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
// When editor validates Game Feature Data asset that owns this action
EDataValidationResult UGfpmAction_AddComponents::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	static const FString NullActorTmpl = TEXT("Null ActorClass at index {0} in ComponentList");
	static const FString NullCompTmpl = TEXT("Null ComponentClass at index {0} in ComponentList");
	int32 EntryIndex = 0;
	for (const FGameFeatureComponentEntry& Entry : ComponentList)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			const FString Formatted = FString::Format(*NullActorTmpl, {FString::FromInt(EntryIndex)});
			Context.AddWarning(FText::FromString(Formatted));
		}
		if (Entry.ComponentClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			const FString Formatted = FString::Format(*NullCompTmpl, {FString::FromInt(EntryIndex)});
			Context.AddWarning(FText::FromString(Formatted));
		}
		++EntryIndex;
	}
	return Result;
}

// When ComponentList entry is added in editor
void UGfpmAction_AddComponents::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName ComponentListName = GET_MEMBER_NAME_CHECKED(ThisClass, ComponentList);
	const bool bIsNewEntry = PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd
	                         && PropertyChangedEvent.GetPropertyName() == ComponentListName;
	if (!bIsNewEntry)
	{
		// Only seed freshly-added entries, manual flag edits including reset to none stay untouched
		return;
	}

	const int32 AddedIndex = PropertyChangedEvent.GetArrayIndex(ComponentListName.ToString());
	if (!ensureMsgf(ComponentList.IsValidIndex(AddedIndex), TEXT("ASSERT: [%i] %hs:\n'AddedIndex' has invalid index!"), __LINE__, __FUNCTION__))
	{
		// ArrayAdd fired but resolved index is out of range, nothing to seed
		return;
	}

	// Recommended defaults avoid CreateComponentOnInstance replacing existing object warning on duplicate adds
	constexpr EGameFrameworkAddComponentFlags DefaultFlags = EGameFrameworkAddComponentFlags::AddUnique | EGameFrameworkAddComponentFlags::UseAutoGeneratedName;
	ComponentList[AddedIndex].AdditionFlags = static_cast<uint8>(DefaultFlags);
}
#endif // WITH_EDITOR

// Per-world component request enqueue against world's framework component manager
void UGfpmAction_AddComponents::AddToWorld(const FWorldContext& WorldContext, FGfpmContextHandles& Handles)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	if (!GameInstance
	    || !World
	    || !World->IsGameWorld())
	{
		// Stale/non-game world context, nothing to add components to
		return;
	}

	UGameFrameworkComponentManager* GFCM = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
	if (!GFCM)
	{
		// GameInstance does not host framework component manager subsystem
		return;
	}

	const ENetMode NetMode = World->GetNetMode();
	const bool bIsServer = NetMode != NM_Client;
	const bool bIsClient = NetMode != NM_DedicatedServer;

	TArray<TSharedPtr<FComponentRequestHandle>>& WorldBucket = Handles.ComponentRequestHandlesByWorld.FindOrAdd(WorldContext.ContextHandle);
	for (const FGameFeatureComponentEntry& Entry : ComponentList)
	{
		const bool bShouldAddRequest = (bIsServer && Entry.bServerComponent) || (bIsClient && Entry.bClientComponent);
		if (!bShouldAddRequest
		    || Entry.ActorClass.IsNull())
		{
			// Net role filter excludes this entry, or actor class is unset, nothing to enqueue
			continue;
		}
		// AddAdditionalAssetBundleData declares ComponentClass in plugin's LoadStateClient/Server bundle, plugin Loading state must have resolved it by now, ensure if not
		const TSubclassOf<UActorComponent> ComponentClass = Entry.ComponentClass.Get();
		if (!ensureMsgf(ComponentClass, TEXT("ASSERT: [%i] %hs:\n'ComponentClass %s' is null, plugin bundle did not preload it"), __LINE__, __FUNCTION__, *Entry.ComponentClass.ToString()))
		{
			continue;
		}
		WorldBucket.Add(GFCM->AddComponentRequest(Entry.ActorClass, ComponentClass, static_cast<EGameFrameworkAddComponentFlags>(Entry.AdditionFlags)));
	}
}

// When GameInstance starts after this action's activation
void UGfpmAction_AddComponents::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	const FWorldContext* WorldContext = GameInstance ? GameInstance->GetWorldContext() : nullptr;
	if (!WorldContext
	    || !ChangeContext.ShouldApplyToWorldContext(*WorldContext))
	{
		// Late-arrival GameInstance has no world context or this activation does not target it
		return;
	}
	FGfpmContextHandles* Handles = ContextHandles.Find(ChangeContext);
	if (!ensureMsgf(Handles, TEXT("ASSERT: [%i] %hs:\n'Handles' is null, ChangeContext never went through activation"), __LINE__, __FUNCTION__))
	{
		return;
	}
	FGfpmScopedWorldContext WorldContextGuard(WorldContext->World());
	AddToWorld(*WorldContext, *Handles);
}
