// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/GameFeatureAction_AddLooseGameplayTags.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Editor.h"
#include "EngineUtils.h"
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddLooseGameplayTags)

#define LOCTEXT_NAMESPACE "GameFeatureAction_AddLooseGameplayTags"

// Called by the Game Features system when the owning feature transitions to Active
void UGameFeatureAction_AddLooseGameplayTags::OnGameFeatureActivated()
{
	if (!ensureMsgf(TaggedActors.IsEmpty() && ExtensionRequestHandles.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'TaggedActors' or 'ExtensionRequestHandles' is not empty, attempting to activate already active feature!"), __LINE__, __FUNCTION__))
	{
		RevokeAllTrackedTags();
	}

	Super::OnGameFeatureActivating();

	if (!ensureMsgf(!WorldAscOwnerActorClass.IsNull(), TEXT("ASSERT: [%i] %hs:\n'WorldAscOwnerActorClass' is not set!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!LooseTags.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'LooseTags' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	OnStartGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &ThisClass::OnGameInstanceStarted);
	OnPostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnPostWorldInit);

#if WITH_EDITOR
	OnEditorMapOpenedHandle = FEditorDelegates::OnMapOpened.AddUObject(this, &ThisClass::OnEditorMapOpened);
#endif

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
void UGameFeatureAction_AddLooseGameplayTags::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FWorldDelegates::OnStartGameInstance.Remove(OnStartGameInstanceHandle);
	OnStartGameInstanceHandle.Reset();

	FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitHandle);
	OnPostWorldInitHandle.Reset();

#if WITH_EDITOR
	FEditorDelegates::OnMapOpened.Remove(OnEditorMapOpenedHandle);
	OnEditorMapOpenedHandle.Reset();
#endif

	RevokeAllTrackedTags();

	Super::OnGameFeatureDeactivating(Context);
}

#if WITH_EDITOR
// Reports configuration errors to the editor's Data Validation system
EDataValidationResult UGameFeatureAction_AddLooseGameplayTags::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (LooseTags.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("EmptyLooseTags", "LooseTags is empty, action will be a no-op"));
	}

	if (WorldAscOwnerActorClass.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NullWorldAscOwnerActorClass", "WorldAscOwnerActorClass is not set, action has no actor receiver to apply tags to"));
	}

	return Result;
}
#endif

// Single dispatch entry per world
void UGameFeatureAction_AddLooseGameplayTags::RegisterForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (World->IsGameWorld()
	    && GameInstance)
	{
		UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
		if (!ensureMsgf(ComponentManager, TEXT("ASSERT: [%i] %hs:\n'ComponentManager' is null!"), __LINE__, __FUNCTION__))
		{
			return;
		}

		const UGameFrameworkComponentManager::FExtensionHandlerDelegate ExtensionDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::OnReceiverExtensionEvent);
		TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(WorldAscOwnerActorClass, ExtensionDelegate);
		ExtensionRequestHandles.Emplace(MoveTemp(ExtensionRequestHandle));
		return;
	}

#if WITH_EDITOR
	if (World->WorldType == EWorldType::Editor)
	{
		UClass* OwnerClass = WorldAscOwnerActorClass.Get();
		if (!ensureMsgf(OwnerClass, TEXT("ASSERT: [%i] %hs:\n'WorldAscOwnerActorClass' '%s' is not loaded; the owning module is expected to load it before activation!"), __LINE__, __FUNCTION__, *WorldAscOwnerActorClass.ToString()))
		{
			return;
		}

		if (!EditorActorSpawnedHandles.Contains(World))
		{
			const FOnActorSpawned::FDelegate SpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::OnActorSpawnedInEditorWorld);
			const FDelegateHandle SpawnedHandle = World->AddOnActorSpawnedHandler(SpawnedDelegate);
			EditorActorSpawnedHandles.Emplace(World, SpawnedHandle);
		}

		for (TActorIterator<AActor> It(World, OwnerClass); It; ++It)
		{
			GrantTagsTo(*It);
		}
	}
#endif
}

// Routes Game Framework Component Manager events to add or remove tag operations
void UGameFeatureAction_AddLooseGameplayTags::OnReceiverExtensionEvent(AActor* Actor, FName EventName)
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
void UGameFeatureAction_AddLooseGameplayTags::OnGameInstanceStarted(UGameInstance* GameInstance)
{
	if (!ensureMsgf(GameInstance, TEXT("ASSERT: [%i] %hs:\n'GameInstance' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	RegisterForWorld(GameInstance->GetWorld());
}

// Called when any world initializes while the feature is Active
void UGameFeatureAction_AddLooseGameplayTags::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues WorldInitializationValues)
{
	RegisterForWorld(World);
}

// Adds LooseTags to the actor's ASC and remembers the actor for cleanup on deactivation
void UGameFeatureAction_AddLooseGameplayTags::GrantTagsTo(AActor* Actor)
{
	if (!ensureMsgf(Actor, TEXT("ASSERT: [%i] %hs:\n'Actor' is null!"), __LINE__, __FUNCTION__)
	    || !Actor->HasAuthority()
	    || TaggedActors.Contains(Actor))
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\nWorldAscOwnerActorClass '%s' does not implement IAbilitySystemInterface or its ASC is null!"), __LINE__, __FUNCTION__, *GetNameSafe(Actor->GetClass())))
	{
		return;
	}

	TaggedActors.Add(Actor);
	ASC->AddLooseGameplayTags(LooseTags, 1, EGameplayTagReplicationState::TagOnly);
}

// Removes LooseTags from the actor's ASC and stops tracking it
void UGameFeatureAction_AddLooseGameplayTags::RevokeTagsFrom(AActor* Actor)
{
	if (!Actor
	    || !Actor->HasAuthority()
	    || !TaggedActors.Contains(Actor))
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (ASC)
	{
		ASC->RemoveLooseGameplayTags(LooseTags, 1, EGameplayTagReplicationState::TagOnly);
	}

	TaggedActors.Remove(Actor);
}

// Removes the granted tags from every tracked actor and clears all per-world handles
void UGameFeatureAction_AddLooseGameplayTags::RevokeAllTrackedTags()
{
	for (auto It = TaggedActors.CreateIterator(); It; ++It)
	{
		const AActor* Actor = It->Get();
		UAbilitySystemComponent* ASC = Actor ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor) : nullptr;
		if (ASC)
		{
			ASC->RemoveLooseGameplayTags(LooseTags, 1, EGameplayTagReplicationState::TagOnly);
		}
		It.RemoveCurrent();
	}

	ExtensionRequestHandles.Empty();

#if WITH_EDITOR
	for (auto It = EditorActorSpawnedHandles.CreateIterator(); It; ++It)
	{
		UWorld* World = It->Key.Get();
		if (World)
		{
			World->RemoveOnActorSpawnedHandler(It->Value);
		}
		It.RemoveCurrent();
	}
#endif
}

#if WITH_EDITOR
// Receiver hook for actors spawned into editor worlds
void UGameFeatureAction_AddLooseGameplayTags::OnActorSpawnedInEditorWorld(AActor* SpawnedActor)
{
	if (!SpawnedActor)
	{
		return;
	}

	const UClass* OwnerClass = WorldAscOwnerActorClass.Get();
	if (!OwnerClass
	    || !SpawnedActor->IsA(OwnerClass))
	{
		return;
	}

	GrantTagsTo(SpawnedActor);
}

// Re-walks editor worlds after a map finishes loading; catches level-loaded receivers that never go through SpawnActor
void UGameFeatureAction_AddLooseGameplayTags::OnEditorMapOpened(const FString& Filename, bool bAsTemplate)
{
	if (!ensureMsgf(GEngine, TEXT("ASSERT: [%i] %hs:\n'GEngine' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (World
		    && World->WorldType == EWorldType::Editor)
		{
			RegisterForWorld(World);
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
