// Copyright (c) Yevhenii Selivanov

#include "ActionObservers/GfpmActionObserver_AddLooseGameplayTags.h"

// GFPM
#include "GameFeatureActions/GfpmAction_AddLooseGameplayTags.h"

// UE
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmActionObserver_AddLooseGameplayTags)

// Identifies the action type this observer handles
TSubclassOf<UGameFeatureAction> UGfpmActionObserver_AddLooseGameplayTags::GetObservedActionClass() const
{
	return UGfpmAction_AddLooseGameplayTags::StaticClass();
}

// When owning plugin finished activating, after its actions ran
void UGfpmActionObserver_AddLooseGameplayTags::OnGameFeatureActivated()
{
	OnEditorMapOpenedHandle = FEditorDelegates::OnMapOpened.AddUObject(this, &ThisClass::OnEditorMapOpened);

	WalkAllEditorWorlds();
}

// When owning plugin begins deactivating
void UGfpmActionObserver_AddLooseGameplayTags::OnGameFeatureDeactivating()
{
	if (OnEditorMapOpenedHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(OnEditorMapOpenedHandle);
		OnEditorMapOpenedHandle.Reset();
	}

	for (auto It = EditorActorSpawnedHandles.CreateIterator(); It; ++It)
	{
		if (const UWorld* World = It->Key.Get())
		{
			World->RemoveOnActorSpawnedHandler(It->Value);
		}
		It.RemoveCurrent();
	}
}

// Walks every editor world to register spawn listeners and grant tags to already-loaded actors
void UGfpmActionObserver_AddLooseGameplayTags::WalkAllEditorWorlds()
{
	if (!ensureMsgf(GEngine, TEXT("ASSERT: [%i] %hs:\n'GEngine' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (World && World->WorldType == EWorldType::Editor)
		{
			RegisterForEditorWorld(World);
		}
	}
}

// Registers spawn listener and grants tags to already-loaded actors of given editor world
void UGfpmActionObserver_AddLooseGameplayTags::RegisterForEditorWorld(UWorld* World)
{
	UGfpmAction_AddLooseGameplayTags* Action = Cast<UGfpmAction_AddLooseGameplayTags>(ObservedAction.Get());
	if (!World || !Action)
	{
		return;
	}

	UClass* OwnerClass = Action->OwnerActor.Get();
	if (!ensureMsgf(OwnerClass, TEXT("ASSERT: [%i] %hs:\n'OwnerActor' '%s' is not loaded; the owning module is expected to load it before activation!"), __LINE__, __FUNCTION__, *Action->OwnerActor.ToString()))
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
		Action->GrantTagsTo(*It);
	}
}

// When an actor is spawned into an editor world
void UGfpmActionObserver_AddLooseGameplayTags::OnActorSpawnedInEditorWorld(AActor* SpawnedActor)
{
	UGfpmAction_AddLooseGameplayTags* Action = Cast<UGfpmAction_AddLooseGameplayTags>(ObservedAction.Get());
	if (!SpawnedActor || !Action)
	{
		return;
	}

	const UClass* OwnerClass = Action->OwnerActor.Get();
	if (!OwnerClass || !SpawnedActor->IsA(OwnerClass))
	{
		return;
	}

	Action->GrantTagsTo(SpawnedActor);
}

// When an editor map finishes loading
void UGfpmActionObserver_AddLooseGameplayTags::OnEditorMapOpened(const FString& Filename, bool bAsTemplate)
{
	WalkAllEditorWorlds();
}
