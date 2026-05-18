// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GfpmLoaderSubsystem.h"

// GFPM
#include "Data/GfpmGameplayTags.h"
#include "Data/GfpmSettings.h"
#include "GfpmUtils.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AsyncMessageId.h"
#include "AsyncMessageSystemBase.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmLoaderSubsystem)

// Returns this subsystem, is checked and will crash if can't be obtained
UGfpmLoaderSubsystem& UGfpmLoaderSubsystem::Get()
{
	UGfpmLoaderSubsystem* Subsystem = GetModularGameFeaturesSubsystem();
	checkf(Subsystem, TEXT("ERROR: [%i] %hs:\n'Subsystem' is null!"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this subsystem
UGfpmLoaderSubsystem* UGfpmLoaderSubsystem::GetModularGameFeaturesSubsystem()
{
	return GEngine ? GEngine->GetEngineSubsystem<UGfpmLoaderSubsystem>() : nullptr;
}

// Returns true if any tag-driven MGF should be active for the current ASC tags but is not yet Active
bool UGfpmLoaderSubsystem::HasPendingTagDrivenActivations() const
{
	const FGameplayTagContainer* OwnedTags = nullptr;
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FGameplayTagContainer>& Pair : AscOwnedTags)
	{
		if (IsAuthoritativeAsc(Pair.Key.Get()))
		{
			OwnedTags = &Pair.Value;
			break;
		}
	}

	if (!OwnedTags)
	{
		// No authoritative ASC tracked yet, treat as pending until readiness is broadcasted
		return true;
	}

	const TMap<FName, FGameplayTagContainer>& FeaturesByTags = UGfpmSettings::Get().GetModularGameFeaturesByTags();
	if (FeaturesByTags.IsEmpty())
	{
		return false;
	}

	for (const TPair<FName, FGameplayTagContainer>& Pair : FeaturesByTags)
	{
		const bool bShouldBeActive = OwnedTags->HasAny(Pair.Value);
		if (bShouldBeActive
		    && !UGfpmUtils::IsModularGameFeatureActive(Pair.Key))
		{
			return true;
		}
	}

	return false;
}

/*********************************************************************************************
 * Tag-Driven Features
 ********************************************************************************************* */

// Snapshots the broadcasting world's ASC tags and registers per-tag listeners
void UGfpmLoaderSubsystem::OnWorldASCReady_Implementation(const FGameplayEventData& Payload)
{
	UAbilitySystemComponent* ASC = const_cast<UAbilitySystemComponent*>(Cast<UAbilitySystemComponent>(Payload.OptionalObject.Get()));
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	FGameplayTagContainer& OwnedSnapshot = AscOwnedTags.FindOrAdd(ASC);
	OwnedSnapshot.Reset();
	ASC->GetOwnedGameplayTags(OwnedSnapshot);

	const FGameplayTagContainer& AllFeatureTags = UGfpmSettings::Get().GetAllModularGameFeatureTags();
	for (const FGameplayTag& Tag : AllFeatureTags)
	{
		ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddWeakLambda(ASC, [ASC](const FGameplayTag ChangedTag, int32 NewCount)
		{
			Get().OnAscTagCountChanged(ChangedTag, NewCount, ASC);
		});
	}

	QueueDeferredRecompute();
}

// Updates the per-ASC tag snapshot and queues a deferred recompute
void UGfpmLoaderSubsystem::OnAscTagCountChanged_Implementation(FGameplayTag ChangedTag, int32 NewCount, UAbilitySystemComponent* SourceAsc)
{
	FGameplayTagContainer* OwnedSnapshot = AscOwnedTags.Find(SourceAsc);
	if (!OwnedSnapshot)
	{
		return;
	}

	if (NewCount > 0)
	{
		OwnedSnapshot->AddTag(ChangedTag);
	}
	else
	{
		OwnedSnapshot->RemoveTag(ChangedTag);
	}

#if WITH_EDITOR
	// First authoritative tag event after a play-session transition releases the hold
	if (bIsAuthorityTransitioning
	    && IsAuthoritativeAsc(SourceAsc))
	{
		bIsAuthorityTransitioning = false;
	}
#endif

	QueueDeferredRecompute();
}

// Coalesces tag-event bursts and keeps MGF activation out of synchronous callbacks
void UGfpmLoaderSubsystem::QueueDeferredRecompute()
{
	if (DeferredRecomputeHandle.IsValid())
	{
		return;
	}

	DeferredRecomputeHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		DeferredRecomputeHandle.Reset();
		ApplyAuthoritativeFeatureSet();
		return false;
	}),
	    0.0f);
}

// In game builds the Game world is authoritative; in editor builds the play world is authoritative during a play session, otherwise the editor world is
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
		return Type == EWorldType::PIE;
	}
#endif

	return Type == EWorldType::Editor
	       || Type == EWorldType::Game;
}

// Aggregates authoritative tags, computes the desired feature set, applies the diff vs the active set
void UGfpmLoaderSubsystem::ApplyAuthoritativeFeatureSet()
{
#if WITH_EDITOR
	if (bIsAuthorityTransitioning
	    || UGfpmUtils::IsAnyWorldPendingNetTravel())
	{
		// Editor<->PIE swap or -game mid client-travel: in editor configs engine creates External Data Layer streaming objects only at world init and only for Active modules,
		// so deactivating plugin here would leave incoming world without streaming object and runtime map switching would break silently later.
		// Skip until new world owns authority and retriggers this recompute
		return;
	}
#endif // WITH_EDITOR

	bool bHasAuthoritativeAsc = false;
	FGameplayTagContainer Aggregate;
	for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FGameplayTagContainer>& Pair : AscOwnedTags)
	{
		const UAbilitySystemComponent* ASC = Pair.Key.Get();
		if (!IsAuthoritativeAsc(ASC))
		{
			continue;
		}
		bHasAuthoritativeAsc = true;
		Aggregate.AppendTags(Pair.Value);
	}

	// Without an authoritative ASC, the aggregate is meaningless; skipping prevents over-deactivating features that the engine activated via .uplugin BuiltInInitialFeatureState before any ASC is registered with this subsystem
	if (!bHasAuthoritativeAsc)
	{
		return;
	}

	const TMap<FName, FGameplayTagContainer>& FeaturesByTags = UGfpmSettings::Get().GetModularGameFeaturesByTags();
	TSet<FName> NewActive;
	NewActive.Reserve(FeaturesByTags.Num());
	TArray<FName> ToActivate;
	TArray<FName> ToDeactivate;
	for (const TPair<FName, FGameplayTagContainer>& Pair : FeaturesByTags)
	{
		const FName& Feature = Pair.Key;
		const bool bShouldBeActive = Aggregate.HasAny(Pair.Value);
		const bool bIsCurrentlyActive = UGfpmUtils::IsModularGameFeatureActive(Feature);

		if (bShouldBeActive)
		{
			NewActive.Add(Feature);
		}

		if (bShouldBeActive
		    && !bIsCurrentlyActive)
		{
			ToActivate.Add(Feature);
		}
		else if (!bShouldBeActive
		         && bIsCurrentlyActive)
		{
			ToDeactivate.Add(Feature);
		}
	}

	if (ToDeactivate.IsEmpty()
	    && ToActivate.IsEmpty())
	{
		return;
	}

	UGfpmUtils::SetModularGameFeaturesActive(false, ToDeactivate);
	UGfpmUtils::SetModularGameFeaturesActive(true, ToActivate);

	ActiveFeatures = MoveTemp(NewActive);
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

	for (auto It = AscOwnedTags.CreateIterator(); It; ++It)
	{
		const UAbilitySystemComponent* ASC = It.Key().Get();
		if (!ASC
		    || ASC->GetWorld() == World)
		{
			It.RemoveCurrent();
		}
	}

#if WITH_EDITOR
	// Map switch in editor: restore all tag-driven features to their .uplugin baseline instead of deactivating
	if (WorldType == EWorldType::Editor)
	{
		TArray<FName> TagDrivenFeatures;
		UGfpmSettings::Get().GetModularGameFeaturesByTags().GetKeys(TagDrivenFeatures);
		UGfpmUtils::RestoreGameFeatureTargetState(TagDrivenFeatures);
		ActiveFeatures.Reset();
		return;
	}

	// Release the transition hold once the last play-typed ASC has been removed so the upcoming recompute can apply the editor authority
	if (bIsAuthorityTransitioning)
	{
		bool bAnyPlayAscRemains = false;
		for (const TPair<TWeakObjectPtr<UAbilitySystemComponent>, FGameplayTagContainer>& Pair : AscOwnedTags)
		{
			const UAbilitySystemComponent* OtherAsc = Pair.Key.Get();
			const UWorld* OtherWorld = OtherAsc ? OtherAsc->GetWorld() : nullptr;
			if (OtherWorld
			    && OtherWorld->WorldType == EWorldType::PIE)
			{
				bAnyPlayAscRemains = true;
				break;
			}
		}
		if (!bAnyPlayAscRemains)
		{
			bIsAuthorityTransitioning = false;
		}
	}
#endif

	QueueDeferredRecompute();
}

#if WITH_EDITOR
void UGfpmLoaderSubsystem::OnPreBeginPIE(bool /*bIsSimulating*/)
{
	bIsAuthorityTransitioning = true;
}

void UGfpmLoaderSubsystem::OnEndPIE(bool /*bIsSimulating*/)
{
	bIsAuthorityTransitioning = true;
}

// Force-deactivate all tag-driven features so the engine emits Unloaded, then queue a recompute to reactivate from current authoritative ASCs
void UGfpmLoaderSubsystem::OnEditorShutdownPIE(bool /*bIsSimulating*/)
{
	bIsAuthorityTransitioning = false;

	TArray<FName> TagDrivenFeatures;
	UGfpmSettings::Get().GetModularGameFeaturesByTags().GetKeys(TagDrivenFeatures);
	UGfpmUtils::RestoreGameFeatureTargetState(TagDrivenFeatures);
	ActiveFeatures.Reset();

	QueueDeferredRecompute();
}
#endif

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Binds world and editor lifecycle delegates
void UGfpmLoaderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this,
	    [this](UWorld* World, const UWorld::InitializationValues /*IVS*/)
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
	FEditorDelegates::PreBeginPIE.AddUObject(this, &ThisClass::OnPreBeginPIE);
	FEditorDelegates::EndPIE.AddUObject(this, &ThisClass::OnEndPIE);
	FEditorDelegates::ShutdownPIE.AddUObject(this, &ThisClass::OnEditorShutdownPIE);
#endif
}

// Unbinds delegates
void UGfpmLoaderSubsystem::Deinitialize()
{
	if (DeferredRecomputeHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DeferredRecomputeHandle);
		DeferredRecomputeHandle.Reset();
	}

	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnPreWorldFinishDestroy.RemoveAll(this);

#if WITH_EDITOR
	FEditorDelegates::PreBeginPIE.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
	FEditorDelegates::ShutdownPIE.RemoveAll(this);
#endif

	Super::Deinitialize();
}
