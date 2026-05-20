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

#if WITH_EDITOR
#include "Editor.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmLoaderSubsystem)

// Returns this subsystem, is checked and will crash if can't be obtained
UGfpmLoaderSubsystem& UGfpmLoaderSubsystem::Get()
{
	UGfpmLoaderSubsystem* Subsystem = GetLoaderSubsystem();
	checkf(Subsystem, TEXT("ERROR: [%i] %hs:\n'Subsystem' is null!"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this subsystem
UGfpmLoaderSubsystem* UGfpmLoaderSubsystem::GetLoaderSubsystem()
{
	return GEngine ? GEngine->GetEngineSubsystem<UGfpmLoaderSubsystem>() : nullptr;
}

// Returns true if any tag-driven GFP should be active for the current ASC tags but is not yet Active
bool UGfpmLoaderSubsystem::HasPendingTagDrivenActivations() const
{
	const UAbilitySystemComponent* AuthAsc = nullptr;
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle>& Pair : TrackedAscs)
	{
		const UAbilitySystemComponent* ASC = Pair.Key.Get();
		if (IsAuthoritativeAsc(ASC))
		{
			AuthAsc = ASC;
			break;
		}
	}

	if (!AuthAsc)
	{
		// No authoritative ASC tracked yet, treat as pending until readiness is broadcasted
		return true;
	}

	if (PluginsByTag.IsEmpty())
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	AuthAsc->GetOwnedGameplayTags(OwnedTags);

	for (const FGameplayTag& Tag : OwnedTags)
	{
		const TArray<FName>* Plugins = PluginsByTag.Find(Tag);
		if (!Plugins)
		{
			continue;
		}
		for (const FName& Plugin : *Plugins)
		{
			if (!UGfpmUtils::IsGameFeaturePluginActive(Plugin))
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

// Tracks the broadcasting ASC via a single generic tag event subscription and schedules an initial apply
void UGfpmLoaderSubsystem::OnWorldASCReady_Implementation(const FGameplayEventData& Payload)
{
	UAbilitySystemComponent* ASC = const_cast<UAbilitySystemComponent*>(Cast<UAbilitySystemComponent>(Payload.OptionalObject.Get()));
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (TrackedAscs.Contains(ASC))
	{
		// Re-broadcast for an already-tracked ASC, just re-apply against the latest tag snapshot
		ScheduleApplyGameFeatures(ASC);
		return;
	}

	const FDelegateHandle Handle = ASC->RegisterGenericGameplayTagEvent().AddWeakLambda(ASC,
	    [ASC](const FGameplayTag ChangedTag, int32 NewCount)
	{
		Get().OnAscTagCountChanged(ChangedTag, NewCount, ASC);
	});
	TrackedAscs.Emplace(ASC, Handle);

	ScheduleApplyGameFeatures(ASC);
}

// Updates the per-ASC tag snapshot and queues a deferred recompute
void UGfpmLoaderSubsystem::OnAscTagCountChanged_Implementation(FGameplayTag ChangedTag, int32 NewCount, UAbilitySystemComponent* SourceAsc)
{
	if (!PluginsByTag.Contains(ChangedTag))
	{
		// Tag does not activate any registered plugin, no work to schedule
		return;
	}

	ScheduleApplyGameFeatures(SourceAsc);
}

// Defers GFPs applying to next tick
void UGfpmLoaderSubsystem::ScheduleApplyGameFeatures(const UObject* WorldContextObject)
{
	const UWorld* World = nullptr;
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle>& Pair : TrackedAscs)
	{
		const UAbilitySystemComponent* ASC = Pair.Key.Get();
		if (IsAuthoritativeAsc(ASC))
		{
			World = ASC->GetWorld();
			break;
		}
	}
	if (!World)
	{
		// Is likely editor launch
		World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	}

	if (!ensureMsgf(World, TEXT("ASSERT: [%i] %hs:\n'World' from WorldContextObject is invalid"), __LINE__, __FUNCTION__)
	    || DeferredRecomputeHandle.IsValid())
	{
		// Is already queued
		return;
	}

	DeferredRecomputeHandle = World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([]
	{
		UGfpmLoaderSubsystem& Loader = Get();
		Loader.DeferredRecomputeHandle.Invalidate();
		Loader.ApplyGameFeatures();
	}));
}

// In game builds Game world is authoritative. In editor builds server-side play world is authoritative during play session excluding client, otherwise editor world is
bool UGfpmLoaderSubsystem::IsAuthoritativeAsc(const UAbilitySystemComponent* ASC) const
{
	if (!ASC)
	{
		return false;
	}

	const UWorld* World = ASC->GetWorld();
	if (!World)
	{
		return false;
	}

	const EWorldType::Type Type = World->WorldType;

#if WITH_EDITOR
	const bool bIsPlaySessionActive = GEditor && GEditor->PlayWorld;
	if (bIsPlaySessionActive)
	{
		// Client play world mirrors authority via replication and lags it, so it must not feed the authoritative aggregate
		return Type == EWorldType::PIE
		       && World->GetNetMode() != NM_Client;
	}
#endif

	return Type == EWorldType::Editor
	       || Type == EWorldType::Game;
}

// Aggregates authoritative tags, computes the desired feature set, applies the diff vs the active set
void UGfpmLoaderSubsystem::ApplyGameFeatures()
{
	if (TrackedAscs.IsEmpty())
	{
		// Nothing to apply
		return;
	}

	bool bHasAuthoritativeAsc = false;
	FGameplayTagContainer Aggregate;
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle>& Pair : TrackedAscs)
	{
		const UAbilitySystemComponent* ASC = Pair.Key.Get();
		if (!IsAuthoritativeAsc(ASC))
		{
			continue;
		}
		bHasAuthoritativeAsc = true;
		FGameplayTagContainer OwnedTags;
		ASC->GetOwnedGameplayTags(OwnedTags);
		Aggregate.AppendTags(OwnedTags);
	}

	// Without an authoritative ASC, the aggregate is meaningless; skipping prevents over-deactivating features that the engine activated via .uplugin BuiltInInitialFeatureState before any ASC is registered with this subsystem
	if (!bHasAuthoritativeAsc)
	{
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
 * World Lifecycle
 ********************************************************************************************* */

// Drains tracked ASCs whose world is dying; editor map switches restore features to baseline, PIE world death recomputes from remaining authority
void UGfpmLoaderSubsystem::OnPreWorldFinishDestroy_Implementation(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const EWorldType::Type WorldType = World->WorldType;

	// Owner world's TimerManager is about to die, cleanup handle
	DeferredRecomputeHandle.Invalidate();

	for (auto It = TrackedAscs.CreateIterator(); It; ++It)
	{
		UAbilitySystemComponent* ASC = It.Key().Get();
		if (ASC && ASC->GetWorld() != World)
		{
			continue;
		}

		if (ASC)
		{
			ASC->RegisterGenericGameplayTagEvent().Remove(It.Value());
		}

		It.RemoveCurrent();
	}

#if WITH_EDITOR
	// Map switch in editor: restore all tag-driven features to their .uplugin baseline instead of deactivating
	if (WorldType == EWorldType::Editor)
	{
		TArray<FName> TagDrivenFeatures;
		GetAllRegisteredPlugins(TagDrivenFeatures);
		UGfpmUtils::RestoreGameFeatureTargetState(TagDrivenFeatures);
		return;
	}
#endif // WITH_EDITOR

	ApplyGameFeatures();
}

#if WITH_EDITOR
// Force-deactivate all tag-driven features so the engine emits Unloaded, then queue a recompute to reactivate from current authoritative ASCs
void UGfpmLoaderSubsystem::OnEditorShutdownPIE(bool /*bIsSimulating*/)
{
	TArray<FName> TagDrivenFeatures;
	GetAllRegisteredPlugins(TagDrivenFeatures);
	UGfpmUtils::RestoreGameFeatureTargetState(TagDrivenFeatures);

	const UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	ScheduleApplyGameFeatures(EditorWorld);
}
#endif // WITH_EDITOR

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Binds world and editor lifecycle delegates
void UGfpmLoaderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this,
	    [this](const UWorld* World, const UWorld::InitializationValues /*IVS*/)
	{
		if (!World)
		{
			return;
		}

		const EWorldType::Type Type = World->WorldType;
		const bool bRelevantWorld =
#if WITH_EDITOR
		    Type == EWorldType::Editor
		    || Type == EWorldType::PIE
		    ||
#endif
		    Type == EWorldType::Game;
		if (!bRelevantWorld)
		{
			return;
		}

		// Listen for Ability System Component to load and unload features
		const TSharedPtr<FAsyncMessageSystemBase> MessageSystem = UAsyncMessageWorldSubsystem::GetSharedMessageSystem(World);
		checkf(MessageSystem, TEXT("ERROR: [%i] %hs:\n'MessageSystem' is null!"), __LINE__, __FUNCTION__);
		MessageSystem->BindListener(FAsyncMessageId(GfpmGameplayTags::Event::WorldASC_Ready), [](const FAsyncMessage& Message)
		{
			const FGameplayEventData* Payload = Message.GetPayloadData<const FGameplayEventData>();
			checkf(Payload, TEXT("ERROR: [%i] %hs:\n'Payload' is null, it's expected to pass Gameplay Event Data as payload!"), __LINE__, __FUNCTION__);
			Get().OnWorldASCReady(*Payload);
		});
	});

	FWorldDelegates::OnPreWorldFinishDestroy.AddUObject(this, &ThisClass::OnPreWorldFinishDestroy);

#if WITH_EDITOR
	FEditorDelegates::ShutdownPIE.AddUObject(this, &ThisClass::OnEditorShutdownPIE);
#endif // WITH_EDITOR
}

// Unbinds delegates and drops every tag subscription
void UGfpmLoaderSubsystem::Deinitialize()
{
	DeferredRecomputeHandle.Invalidate();

	// Drop every tag subscription and clears tracking
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle>& Pair : TrackedAscs)
	{
		if (UAbilitySystemComponent* ASC = Pair.Key.Get())
		{
			ASC->RegisterGenericGameplayTagEvent().Remove(Pair.Value);
		}
	}
	TrackedAscs.Empty();

	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnPreWorldFinishDestroy.RemoveAll(this);

#if WITH_EDITOR
	FEditorDelegates::ShutdownPIE.RemoveAll(this);
#endif // WITH_EDITOR

	Super::Deinitialize();
}

// Collects the union of plugin names appearing across every entry of PluginsByTag
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

// Registers or replaces activation entry of owning Game Feature Plugin resolved from given GameFeatureData
void UGfpmLoaderSubsystem::RegisterGameFeaturePluginActivation(const UGameFeatureData* GameFeatureData, const FGameplayTagContainer& ActivationTags)
{
	const FName PluginName = UGfpmUtils::GetModuleNameByAsset(GameFeatureData);
	if (!ensureMsgf(PluginName.IsValid(), TEXT("ASSERT: [%i] %hs:\n'PluginName' resolved from GameFeatureData is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Clean stale tag entries from prior registration so re-registering with different tag set does not leak old bindings
	UnregisterGameFeaturePluginActivation(GameFeatureData);

	for (const FGameplayTag& Tag : ActivationTags)
	{
		PluginsByTag.FindOrAdd(Tag).AddUnique(PluginName);
	}

	// Re-apply through any tracked ASC's world
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle>& Pair : TrackedAscs)
	{
		if (const UAbilitySystemComponent* ASC = Pair.Key.Get())
		{
			ScheduleApplyGameFeatures(ASC);
			break;
		}
	}
}

// Drops activation entry of owning Game Feature Plugin resolved from given GameFeatureData
void UGfpmLoaderSubsystem::UnregisterGameFeaturePluginActivation(const UGameFeatureData* GameFeatureData)
{
	const FName PluginName = UGfpmUtils::GetModuleNameByAsset(GameFeatureData);
	if (PluginName.IsNone())
	{
		// GameFeatureData resolves to no plugin module, nothing tracked under it
		return;
	}

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
