// Copyright (c) Yevhenii Selivanov

#include "FTGEditorPreviewSubsystem.h"
//---
#include "FTGComponent.h"
#include "FTGEditorUtils.h"
#include "GeneratedMap.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "InstancedStaticMeshActor.h"
//---
#include "Engine/World.h"
//---
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
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld() // Only preview in editor, not in PIE
		|| IsValid(FootTrailGeneratorInternal)) // skip if already initialized
	{
		return;
	}

	UGeneratedMapSubsystem* GeneratedMapSubsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem(World);
	if (!GeneratedMapSubsystem)
	{
		// Might be null in temporary editor worlds, ignore
		return;
	}

	GeneratedMapSubsystem->OnGeneratedMapReady.AddUniqueDynamic(this, &ThisClass::OnGeneratedMapReady);
	if (AGeneratedMap* GeneratedMap = AGeneratedMap::GetGeneratedMap(World))
	{
		OnGeneratedMapReady(GeneratedMap);
	}
}

// Is used to destroy the foot trails generator
void UFTGEditorPreviewSubsystem::OnEndPlay(UWorld* World, bool bArg, bool bCond)
{
	if (IsValid(FootTrailGeneratorInternal))
	{
		FootTrailGeneratorInternal->DestroyComponent();
		FootTrailGeneratorInternal = nullptr;
	}

	if (UGeneratedMapSubsystem* GeneratedMapSubsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem(World))
	{
		GeneratedMapSubsystem->OnGeneratedMapReady.RemoveAll(this);
	}
}

/// Called when Generated Map is initialized and ready to be used, is also called in editor
void UFTGEditorPreviewSubsystem::OnGeneratedMapReady(class AGeneratedMap* GeneratedMap)
{
	if (IsValid(FootTrailGeneratorInternal))
	{
		// Is already initialized
		return;
	}

	const TSubclassOf<UFTGComponent> ComponentClass = UFTGEditorUtils::GetFootTrailsComponentClass();
	if (!ensureMsgf(ComponentClass, TEXT("ASSERT: [%i] %hs:\n'ComponentClass' is not found! Make sure '%s' class is assigned in MGF data asset"), __LINE__, __FUNCTION__, *UFTGComponent::StaticClass()->GetName()))
	{
		return;
	}

	checkf(GeneratedMap, TEXT("ERROR: [%i] %hs:\n'GeneratedMap' is null!"), __LINE__, __FUNCTION__);
	FootTrailGeneratorInternal = NewObject<UFTGComponent>(GeneratedMap, ComponentClass);
	FootTrailGeneratorInternal->RegisterComponent();
	FootTrailGeneratorInternal->InitOnce();

	// Hide editor-only version of trails in PIE
	AInstancedStaticMeshActor* InstancedFootTrailsActor = FootTrailGeneratorInternal->GetInstancedStaticMeshActor();
	checkf(InstancedFootTrailsActor, TEXT("ERROR: [%i] %hs:\n'InstancedFootTrailsActor' is null!"), __LINE__, __FUNCTION__);
	InstancedFootTrailsActor->SetActorHiddenInGame(true);
}
