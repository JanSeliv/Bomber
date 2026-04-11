// Copyright (c) Yevhenii Selivanov

#include "Subsystems/MyWorldPartitionEditorSubsystem.h"

// UE
#include "Engine/World.h"
#include "WorldPartition/LoaderAdapter/LoaderAdapterShape.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionEditorLoaderAdapter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MyWorldPartitionEditorSubsystem)

// Binds to world initialization delegate to auto-load WP regions
void UMyWorldPartitionEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnPostWorldInitialization);
}

// Unbinds from world initialization delegate
void UMyWorldPartitionEditorSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);

	Super::Deinitialize();
}

// Binds to per-world WP initialized event for editor worlds
void UMyWorldPartitionEditorSubsystem::OnPostWorldInitialization(UWorld* World, const struct FWorldInitializationValues IVS)
{
	if (!World
	    || World->WorldType != EWorldType::Editor
	    || IsRunningCommandlet())
	{
		return;
	}

	UWorldPartition* WorldPartition = World->GetWorldPartition();
	const bool bAlreadyInitialized = WorldPartition && WorldPartition->IsInitialized();
	if (bAlreadyInitialized)
	{
		OnWorldPartitionInitialized(WorldPartition);
		return;
	}

	World->OnWorldPartitionInitialized().AddUObject(this, &ThisClass::OnWorldPartitionInitialized);
}

// Auto-loads all WP regions on first open when no saved regions exist
void UMyWorldPartitionEditorSubsystem::OnWorldPartitionInitialized(UWorldPartition* InWorldPartition)
{
	if (!ensureMsgf(InWorldPartition, TEXT("ASSERT: [%i] %hs:\n'InWorldPartition' is not valid"), __LINE__, __FUNCTION__)
	    || InWorldPartition->HasLoadedUserCreatedRegions())
	{
		// Is not first time load
		return;
	}

	// First-time open with no saved regions: load entire editor world bounds
	const FBox EditorBounds = InWorldPartition->GetEditorWorldBounds();
	UWorld* World = InWorldPartition->GetWorld();
	if (!ensureMsgf(EditorBounds.IsValid, TEXT("ASSERT: [%i] %hs:\n'EditorBounds' is not valid for initialized World Partition"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(World, TEXT("ASSERT: [%i] %hs:\n'World' is not valid"), __LINE__, __FUNCTION__))
	{
		return;
	}

	UWorldPartitionEditorLoaderAdapter* Adapter = InWorldPartition->CreateEditorLoaderAdapter<FLoaderAdapterShape>(World, EditorBounds, TEXT("Auto Loaded"));
	IWorldPartitionActorLoaderInterface::ILoaderAdapter* LoaderAdapter = Adapter ? Adapter->GetLoaderAdapter() : nullptr;
	if (!ensureMsgf(LoaderAdapter, TEXT("ASSERT: [%i] %hs:\n'LoaderAdapter' is not valid"), __LINE__, __FUNCTION__))
	{
		return;
	}

	LoaderAdapter->SetUserCreated(true);
	LoaderAdapter->Load();
}