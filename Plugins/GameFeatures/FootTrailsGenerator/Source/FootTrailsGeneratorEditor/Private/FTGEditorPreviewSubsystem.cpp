// Copyright (c) Yevhenii Selivanov

#include "FTGEditorPreviewSubsystem.h"

// FTG
#include "FTGComponent.h"
#include "FTGEditorUtils.h"

/// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGeneratedMapSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"

// UE
#include "Engine/World.h"
#include "InstancedStaticMeshActor.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FTGEditorPreviewSubsystem)

// Is called when the subsystem is initialized
void UFTGEditorPreviewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnBeginPlay);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnEndPlay);
}

// Is called when the subsystem is deinitialized
void UFTGEditorPreviewSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

	Super::Deinitialize();
}

// Is used to initialize the foot trails generator
void UFTGEditorPreviewSubsystem::OnBeginPlay(UWorld* World, FWorldInitializationValues WorldInitializationValues)
{
	if (!UUtilsLibrary::IsEditorNotPieWorld() // Only preview in editor, not in PIE
	    || IsValid(FootTrailGenerator)) // skip if already initialized
	{
		return;
	}

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GeneratedMap_Ready, this, &ThisClass::OnGeneratedMapReady);
}

// Is used to destroy the foot trails generator
void UFTGEditorPreviewSubsystem::OnEndPlay(UWorld* World, bool bArg, bool bCond)
{
	if (IsValid(FootTrailGenerator))
	{
		FootTrailGenerator->DestroyComponent();
		FootTrailGenerator = nullptr;
	}
}

/// Called when Generated Map is initialized and its data assets are loaded, is also called in editor
void UFTGEditorPreviewSubsystem::OnGeneratedMapReady_Implementation(const FGameplayEventData& Payload)
{
	if (IsValid(FootTrailGenerator))
	{
		// Is already initialized
		return;
	}

	const TSubclassOf<UFTGComponent> ComponentClass = UFTGEditorUtils::GetFootTrailsComponentClass();
	if (!ensureMsgf(ComponentClass, TEXT("ASSERT: [%i] %hs:\n'ComponentClass' is not found! Make sure '%s' class is assigned in MGF data asset"), __LINE__, __FUNCTION__, *UFTGComponent::StaticClass()->GetName()))
	{
		return;
	}

	FootTrailGenerator = NewObject<UFTGComponent>(ABmrGeneratedMap::GetGeneratedMap(), ComponentClass, NAME_None, RF_Transient);
	FootTrailGenerator->RegisterComponent();
	FootTrailGenerator->InitOnce();

	// Hide editor-only version of trails in PIE
	AInstancedStaticMeshActor* InstancedFootTrailsActor = FootTrailGenerator->GetInstancedStaticMeshActor();
	checkf(InstancedFootTrailsActor, TEXT("ERROR: [%i] %hs:\n'InstancedFootTrailsActor' is null!"), __LINE__, __FUNCTION__);
	InstancedFootTrailsActor->SetActorHiddenInGame(true);
}
