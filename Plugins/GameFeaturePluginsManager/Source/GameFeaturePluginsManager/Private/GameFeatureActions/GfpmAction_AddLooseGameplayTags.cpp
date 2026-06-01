// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_AddLooseGameplayTags.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAction_AddLooseGameplayTags)

#define LOCTEXT_NAMESPACE "GameFeatureAction_AddLooseGameplayTags"

// Called by the Game Features system when the owning feature transitions to Active
void UGfpmAction_AddLooseGameplayTags::OnGameFeatureActivated()
{
	if (!ensureMsgf(!OnStartGameInstanceHandle.IsValid(), TEXT("ASSERT: [%i] %hs:\n'OnStartGameInstanceHandle' is still valid, attempting to activate already active feature!"), __LINE__, __FUNCTION__))
	{
		// Recover from feature activated twice without deactivation in between
		RevokeAllTrackedTags();
	}

	Super::OnGameFeatureActivating();

	if (!ensureMsgf(!OwnerActor.IsNull(), TEXT("ASSERT: [%i] %hs:\n'OwnerActor' is not set!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!LooseTags.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'LooseTags' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	OnStartGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &ThisClass::OnGameInstanceStarted);
	OnPostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnPostWorldInit);

	if (!ensureMsgf(GEngine, TEXT("ASSERT: [%i] %hs:\n'GEngine' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		RegisterForWorld(WorldContext.World());
	}
}

// Called by the Game Features system when the owning feature is leaving the Active state
void UGfpmAction_AddLooseGameplayTags::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FWorldDelegates::OnStartGameInstance.Remove(OnStartGameInstanceHandle);
	OnStartGameInstanceHandle.Reset();

	FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitHandle);
	OnPostWorldInitHandle.Reset();

	RevokeAllTrackedTags();

	Super::OnGameFeatureDeactivating(Context);
}

#if WITH_EDITOR
// Reports configuration errors to the editor's Data Validation system
EDataValidationResult UGfpmAction_AddLooseGameplayTags::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (LooseTags.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("EmptyLooseTags", "LooseTags is empty, action will be a no-op"));
	}

	if (OwnerActor.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NullWorldAscOwnerActorClass", "OwnerActor is not set, action has no actor receiver to apply tags to"));
	}

	return Result;
}
#endif

// Single dispatch entry per world
void UGfpmAction_AddLooseGameplayTags::RegisterForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (World->IsGameWorld()
	    && GameInstance)
	{
		UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
		if (!ensureMsgf(ComponentManager, TEXT("ASSERT: [%i] %hs:\n'ComponentManager' is null!"), __LINE__, __FUNCTION__))
		{
			return;
		}

		FGfpmLooseTagsWorldData& WorldData = PerWorldData.FindOrAdd(World);
		if (!WorldData.ExtensionRequestHandles.IsEmpty())
		{
			// World already registered, RegisterForWorld is reachable more than once per world via OnPostWorldInit and OnGameInstanceStarted
			return;
		}

		const UGameFrameworkComponentManager::FExtensionHandlerDelegate ExtensionDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::OnReceiverExtensionEvent);
		TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(OwnerActor, ExtensionDelegate);
		WorldData.ExtensionRequestHandles.Emplace(MoveTemp(ExtensionRequestHandle));
	}
}

// Routes Game Framework Component Manager events to add or remove tag operations
void UGfpmAction_AddLooseGameplayTags::OnReceiverExtensionEvent(AActor* Actor, FName EventName)
{
	const bool bIsAddEvent = EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded
	                         || EventName == UGameFrameworkComponentManager::NAME_ReceiverAdded;
	const bool bIsRemoveEvent = EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved
	                            || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved;

	if (bIsAddEvent)
	{
		GrantTagsTo(Actor);
	}
	else if (bIsRemoveEvent)
	{
		RevokeTagsFrom(Actor);
	}
}

// Called when a new game instance starts up while the feature is Active
void UGfpmAction_AddLooseGameplayTags::OnGameInstanceStarted(UGameInstance* GameInstance)
{
	if (!ensureMsgf(GameInstance, TEXT("ASSERT: [%i] %hs:\n'GameInstance' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	RegisterForWorld(GameInstance->GetWorld());
}

// Called when any world initializes while the feature is Active
void UGfpmAction_AddLooseGameplayTags::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues WorldInitializationValues)
{
	RegisterForWorld(World);
}

// Adds LooseTags to the actor's ASC and remembers the actor for cleanup on deactivation
void UGfpmAction_AddLooseGameplayTags::GrantTagsTo(AActor* Actor)
{
	if (!ensureMsgf(Actor, TEXT("ASSERT: [%i] %hs:\n'Actor' is null!"), __LINE__, __FUNCTION__)
	    || !Actor->HasAuthority())
	{
		return;
	}

	UWorld* World = Actor->GetWorld();
	if (!World)
	{
		// Actor outside any world, cannot bucket its tracking
		return;
	}

	// Bucket tracking by the actor's world, this action is a shared instance servicing every world
	FGfpmLooseTagsWorldData& WorldData = PerWorldData.FindOrAdd(World);
	if (WorldData.TaggedActors.Contains(Actor))
	{
		// Already granted in this world
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\nWorldAscOwnerActorClass '%s' does not implement IAbilitySystemInterface or its ASC is null!"), __LINE__, __FUNCTION__, *GetNameSafe(Actor->GetClass())))
	{
		return;
	}

	// Listen to remove self tag if set; subscribe via generic tag event before granting, so future child tags trigger removal
	if (bIsExclusiveTag
	    && !WorldData.ExclusiveTagSubscriptions.Contains(ASC))
	{
		const TWeakObjectPtr<AActor> WeakActor(Actor);
		const FDelegateHandle Handle = ASC->RegisterGenericGameplayTagEvent().AddUObject(this, &ThisClass::OnAnyExclusiveAscTagChanged, WeakActor);
		WorldData.ExclusiveTagSubscriptions.Emplace(ASC, Handle);
	}

	// Apply desired tags; always grant first so external observers (GFP loader) see the add event even when we are about to remove for a pre-existing child
	WorldData.TaggedActors.Add(Actor);
	ASC->AddLooseGameplayTags(LooseTags, 1, EGameplayTagReplicationState::TagOnly);

	// Snapshot scan after grant: generic event only fires on count change, so pre-existing children need a one-shot check. If found, remove own tags so loader picks up the remove event and deactivates the GFP
	if (bIsExclusiveTag)
	{
		FGameplayTagContainer ExistingTags;
		ASC->GetOwnedGameplayTags(ExistingTags);
		for (const FGameplayTag& ExistingTag : ExistingTags)
		{
			if (LooseTags.HasTagExact(ExistingTag))
			{
				continue;
			}

			const FGameplayTag ExistingParent = ExistingTag.RequestDirectParent();
			if (!ExistingParent.IsValid())
			{
				continue;
			}

			for (const FGameplayTag& OwnTag : LooseTags)
			{
				if (OwnTag.RequestDirectParent() != ExistingParent)
				{
					continue;
				}

				RevokeTagsFrom(Actor);
				return;
			}
		}
	}
}

// Removes LooseTags from the actor's ASC and stops tracking it
void UGfpmAction_AddLooseGameplayTags::RevokeTagsFrom(AActor* Actor)
{
	UWorld* World = Actor ? Actor->GetWorld() : nullptr;
	FGfpmLooseTagsWorldData* WorldData = World ? PerWorldData.Find(World) : nullptr;
	if (!Actor
	    || !Actor->HasAuthority()
	    || !WorldData
	    || !WorldData->TaggedActors.Contains(Actor))
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (ASC)
	{
		if (const FDelegateHandle* Handle = WorldData->ExclusiveTagSubscriptions.Find(ASC))
		{
			ASC->RegisterGenericGameplayTagEvent().Remove(*Handle);
			WorldData->ExclusiveTagSubscriptions.Remove(ASC);
		}

		ASC->RemoveLooseGameplayTags(LooseTags, 1, EGameplayTagReplicationState::TagOnly);
	}

	WorldData->TaggedActors.Remove(Actor);
}

// Removes the granted tags from every tracked actor across all worlds
void UGfpmAction_AddLooseGameplayTags::RevokeAllTrackedTags()
{
	for (TPair<TWeakObjectPtr<UWorld>, FGfpmLooseTagsWorldData>& WorldPair : PerWorldData)
	{
		RevokeWorldData(WorldPair.Value);
	}

	PerWorldData.Empty();
}

// Removes the granted tags and clears handles for a single world's tracked data
void UGfpmAction_AddLooseGameplayTags::RevokeWorldData(FGfpmLooseTagsWorldData& WorldData)
{
	for (auto It = WorldData.ExclusiveTagSubscriptions.CreateIterator(); It; ++It)
	{
		UAbilitySystemComponent* ASC = It->Key.Get();
		if (ASC)
		{
			ASC->RegisterGenericGameplayTagEvent().Remove(It->Value);
		}
		It.RemoveCurrent();
	}

	for (auto It = WorldData.TaggedActors.CreateIterator(); It; ++It)
	{
		const AActor* Actor = It->Get();
		UAbilitySystemComponent* ASC = Actor ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor) : nullptr;
		// Only remove tags ASC still owns, since teardown or exclusive-tag events may have already cleared them
		if (ASC
		    && ASC->GetOwnedGameplayTags().HasAnyExact(LooseTags))
		{
			ASC->RemoveLooseGameplayTags(LooseTags, 1, EGameplayTagReplicationState::TagOnly);
		}
		It.RemoveCurrent();
	}

	WorldData.ExtensionRequestHandles.Empty();
}

// Hook for any ASC tag count change; removes own tags when another child tag sharing the same direct parent as one of LooseTags is added
void UGfpmAction_AddLooseGameplayTags::OnAnyExclusiveAscTagChanged(FGameplayTag ChangedTag, int32 NewCount, TWeakObjectPtr<AActor> WeakActor)
{
	AActor* Actor = WeakActor.Get();
	if (NewCount <= 0
	    || LooseTags.HasTagExact(ChangedTag))
	{
		return;
	}

	const FGameplayTag ChangedParent = ChangedTag.RequestDirectParent();
	if (!ChangedParent.IsValid())
	{
		return;
	}

	bool bIsChild = false;
	for (const FGameplayTag& OwnTag : LooseTags)
	{
		if (OwnTag.RequestDirectParent() == ChangedParent)
		{
			bIsChild = true;
			break;
		}
	}

	const UWorld* World = Actor ? Actor->GetWorld() : nullptr;
	const FGfpmLooseTagsWorldData* WorldData = World ? PerWorldData.Find(World) : nullptr;
	if (!bIsChild
	    || !WorldData
	    || !WorldData->TaggedActors.Contains(Actor))
	{
		return;
	}

	RevokeTagsFrom(Actor);
}

#undef LOCTEXT_NAMESPACE
