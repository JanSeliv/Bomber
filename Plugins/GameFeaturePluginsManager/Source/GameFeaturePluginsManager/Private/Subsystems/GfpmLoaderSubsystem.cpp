// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GfpmLoaderSubsystem.h"

// GFPM
#include "Data/GfpmGameplayTags.h"
#include "GfpmUtils.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AsyncMessageId.h"
#include "AsyncMessageSystemBase.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeatureData.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmLoaderSubsystem)

// Returns this subsystem for current play world, checked. Crashes if unavailable
UGfpmLoaderSubsystem& UGfpmLoaderSubsystem::Get()
{
	UGfpmLoaderSubsystem* Subsystem = GetLoaderSubsystem();
	checkf(Subsystem, TEXT("ERROR: [%i] %hs:\n'Subsystem' is null!"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns this subsystem for current play world or nullptr
UGfpmLoaderSubsystem* UGfpmLoaderSubsystem::GetLoaderSubsystem()
{
	const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
#if WITH_EDITOR
	if (!World && GEngine)
	{
		// Editor build with no active play world yet: fall back to editor world so registrations at engine init resolve
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.WorldType == EWorldType::Editor && WorldContext.World())
			{
				World = WorldContext.World();
				break;
			}
		}
	}
#endif // WITH_EDITOR
	return World ? World->GetSubsystem<UGfpmLoaderSubsystem>() : nullptr;
}

// Whether any tag-driven GFP is pending activation despite owning required tags on tracked ASC
bool UGfpmLoaderSubsystem::HasPendingTagDrivenActivations() const
{
	if (!TrackedAsc)
	{
		// No ASC tracked yet, treat as pending until readiness is broadcasted
		return true;
	}

	if (PluginsByTag.IsEmpty())
	{
		// No registered plugins, nothing can be pending
		return false;
	}

	if (DeferredRecomputeHandle.IsValid())
	{
		// Apply pass already queued for next tick, tag-driven states are about to change
		return true;
	}

	FGameplayTagContainer OwnedTags;
	TrackedAsc->GetOwnedGameplayTags(OwnedTags);

	for (const FGameplayTag& Tag : OwnedTags)
	{
		const TArray<FName>* Plugins = PluginsByTag.Find(Tag);
		if (!Plugins)
		{
			continue;
		}
		for (const FName& Plugin : *Plugins)
		{
			const bool bIsActive = UGfpmUtils::IsGameFeaturePluginActive(Plugin);
			const bool bIsTransitioning = UGfpmUtils::IsGameFeaturePluginActive(Plugin, /*bCheckForPending=*/true);
			if (bIsTransitioning
			    && !bIsActive)
			{
				return true;
			}
		}
	}

	return false;
}

/*********************************************************************************************
 * Tag-Driven Features
 ********************************************************************************************* */

// When this world's ASC becomes available and ready to broadcast tags
void UGfpmLoaderSubsystem::OnWorldASCReady_Implementation(const FGameplayEventData& Payload)
{
	UAbilitySystemComponent* ASC = const_cast<UAbilitySystemComponent*>(Cast<UAbilitySystemComponent>(Payload.OptionalObject.Get()));
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (ASC->GetWorld() != GetWorld())
	{
		// Per-world subsystem expects ASC to belong to its own world. Ignore foreign ASCs
		return;
	}

	if (TrackedAsc == ASC)
	{
		// Re-broadcast for already-tracked ASC, no proof tags exist yet so guard against empty-aggregate deactivation
		ScheduleApplyGameFeatures(/*bAllowEmptyAggregate=*/false);
		return;
	}

	// Replace any prior ASC tracking (e.g. seamless travel swapping ASC) before installing new binding
	if (TrackedAsc && TagEventHandle.IsValid())
	{
		TrackedAsc->RegisterGenericGameplayTagEvent().Remove(TagEventHandle);
		TagEventHandle.Reset();
	}

	TrackedAsc = ASC;
	TagEventHandle = ASC->RegisterGenericGameplayTagEvent().AddWeakLambda(this,
	    [this](const FGameplayTag ChangedTag, int32 NewCount)
	{
		OnAscTagCountChanged(ChangedTag, NewCount);
	});

	// First binding, ASC may not have its tags populated yet, conservative startup path
	ScheduleApplyGameFeatures(/*bAllowEmptyAggregate=*/false);
}

// When tracked ASC's tag count changes
void UGfpmLoaderSubsystem::OnAscTagCountChanged_Implementation(FGameplayTag ChangedTag, int32 NewCount)
{
	if (!PluginsByTag.Contains(ChangedTag))
	{
		// Tag does not activate any registered plugin, no work to schedule
		return;
	}

	// Explicit tag-change event, empty aggregate at apply time means relevant tag was just removed and must deactivate
	ScheduleApplyGameFeatures(/*bAllowEmptyAggregate=*/true);
}

// Defers GFPs applying to next tick, coalescing burst tag events into one pass once tags settle
void UGfpmLoaderSubsystem::ScheduleApplyGameFeatures(bool bAllowEmptyAggregate)
{
	const UWorld* World = GetWorld();
	if (!ensureMsgf(World, TEXT("ASSERT: [%i] %hs:\n'World' is null!"), __LINE__, __FUNCTION__)
	    || DeferredRecomputeHandle.IsValid())
	{
		// Is already queued
		return;
	}

	DeferredRecomputeHandle = World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, bAllowEmptyAggregate]
	{
		DeferredRecomputeHandle.Invalidate();
		ApplyGameFeatures(bAllowEmptyAggregate);
	}));
}

// Recomputes desired feature set from this world's ASC tags and applies activation/deactivation delta
void UGfpmLoaderSubsystem::ApplyGameFeatures(bool bAllowEmptyAggregate)
{
	if (!TrackedAsc)
	{
		// Without tracked ASC for this world, aggregate is meaningless, skipping prevents
		// over-deactivating features that engine activated via .uplugin BuiltInInitialFeatureState
		return;
	}

	FGameplayTagContainer Aggregate;
	TrackedAsc->GetOwnedGameplayTags(Aggregate);

	if (Aggregate.IsEmpty() && !bAllowEmptyAggregate)
	{
		// Caller has no proof tags ever existed, empty aggregate here would deactivate every plugin that came up via .uplugin BuiltInInitialFeatureState
		return;
	}

	// Project the inverted index through the aggregate to compute the should-activate set
	TSet<FName> ShouldBeActive;
	for (const FGameplayTag& Tag : Aggregate)
	{
		const TArray<FName>* Plugins = PluginsByTag.Find(Tag);
		if (!Plugins)
		{
			continue;
		}
		for (const FName& Plugin : *Plugins)
		{
			ShouldBeActive.Add(Plugin);
		}
	}

	TArray<FName> AllRegistered;
	GetAllRegisteredPlugins(AllRegistered);

	TArray<FName> ToActivate;
	TArray<FName> ToDeactivate;
	for (const FName& Plugin : AllRegistered)
	{
		const bool bShouldBeActive = ShouldBeActive.Contains(Plugin);
		const bool bIsCurrentlyActive = UGfpmUtils::IsGameFeaturePluginActive(Plugin);

		if (bShouldBeActive
		    && !bIsCurrentlyActive)
		{
			ToActivate.Add(Plugin);
		}
		else if (!bShouldBeActive
		         && bIsCurrentlyActive)
		{
			ToDeactivate.Add(Plugin);
		}
	}

	if (ToDeactivate.IsEmpty()
	    && ToActivate.IsEmpty())
	{
		return;
	}

	UGfpmUtils::SetGameFeaturePluginsActive(false, ToDeactivate);
	UGfpmUtils::SetGameFeaturePluginsActive(true, ToActivate);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// When subsystem initializes
void UGfpmLoaderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UWorld* World = GetWorld();
	if (!ensureMsgf(World, TEXT("ASSERT: [%i] %hs:\n'World' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Listen for this world's Ability System Component to broadcast its readiness
	const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
	checkf(MessageSystem, TEXT("ERROR: [%i] %hs:\n'MessageSystem' is null!"), __LINE__, __FUNCTION__);
	MessageSystem->BindListener(FAsyncMessageId(GfpmGameplayTags::Event::WorldASC_Ready), [WeakThis = TWeakObjectPtr<ThisClass>(this)](const FAsyncMessage& Message)
	{
		ThisClass* StrongThis = WeakThis.Get();
		if (!StrongThis)
		{
			// Subsystem was destroyed before the message arrived
			return;
		}
		const FGameplayEventData* Payload = Message.GetPayloadData<const FGameplayEventData>();
		checkf(Payload, TEXT("ERROR: [%i] %hs:\n'Payload' is null, it's expected to pass Gameplay Event Data as payload!"), __LINE__, __FUNCTION__);
		StrongThis->OnWorldASCReady(*Payload);
	});
}

// When subsystem is destroyed
void UGfpmLoaderSubsystem::Deinitialize()
{
	DeferredRecomputeHandle.Invalidate();

	if (TrackedAsc && TagEventHandle.IsValid())
	{
		TrackedAsc->RegisterGenericGameplayTagEvent().Remove(TagEventHandle);
	}
	TagEventHandle.Reset();
	TrackedAsc = nullptr;

	Super::Deinitialize();
}

// Collects union of plugin names appearing across every entry of this instance's registry
void UGfpmLoaderSubsystem::GetAllRegisteredPlugins(TArray<FName>& OutNames) const
{
	OutNames.Reset();
	for (const TPair<FGameplayTag, TArray<FName>>& Pair : PluginsByTag)
	{
		for (const FName& Plugin : Pair.Value)
		{
			OutNames.AddUnique(Plugin);
		}
	}
}

// Registers or replaces this world's activation entry of owning Game Feature Plugin resolved from given GameFeatureData, then schedules apply
void UGfpmLoaderSubsystem::RegisterGameFeaturePluginActivation(const UGameFeatureData* GameFeatureData, const FGameplayTagContainer& ActivationTags)
{
	const FName PluginName = UGfpmUtils::GetModuleNameByAsset(GameFeatureData);
	if (!ensureMsgf(PluginName.IsValid(), TEXT("ASSERT: [%i] %hs:\n'PluginName' resolved from GameFeatureData is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Clean stale tag entries from prior registration so re-registering with different tag set does not leak old bindings
	RemovePluginFromRegistry(PluginName);
	for (const FGameplayTag& Tag : ActivationTags)
	{
		PluginsByTag.FindOrAdd(Tag).AddUnique(PluginName);
	}

	// Late registration with no proof tags exist yet, conservative startup path semantics
	ScheduleApplyGameFeatures(/*bAllowEmptyAggregate=*/false);
}

// Drops this world's activation entry of owning Game Feature Plugin resolved from given GameFeatureData
void UGfpmLoaderSubsystem::UnregisterGameFeaturePluginActivation(const UGameFeatureData* GameFeatureData)
{
	const FName PluginName = UGfpmUtils::GetModuleNameByAsset(GameFeatureData);
	if (PluginName.IsNone())
	{
		// GameFeatureData resolves to no plugin module, nothing tracked under it
		return;
	}

	RemovePluginFromRegistry(PluginName);
}

// Removes every PluginsByTag entry pointing at given plugin name from this instance only
void UGfpmLoaderSubsystem::RemovePluginFromRegistry(FName PluginName)
{
	for (auto It = PluginsByTag.CreateIterator(); It; ++It)
	{
		TArray<FName>& Plugins = It.Value();
		Plugins.RemoveSingle(PluginName);
		if (Plugins.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}
