// Copyright (c) Yevhenii Selivanov

#include "Components/PasPredictionComponent.h"

// PAS
#include "Components/PasSurrounderLogicComponent.h"
#include "Data/PasDataAsset.h"
#include "PasBlueprintFunctionLibrary.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"

// UE
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PasPredictionComponent)

UPasPredictionComponent::UPasPredictionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Fires after each wall step on sibling surrounder
void UPasPredictionComponent::OnWallSpawned_Implementation()
{
	if (!Surrounder)
	{
		// Sibling surrounder vanished, nothing to predict from
		return;
	}
	const FPasCellDataOnSide MyCellData = Surrounder->GetCurrentCellData();
	const TArray<FPasCellDataOnSide> Next = UPasBlueprintFunctionLibrary::GetNextCells(MyCellData, GetPredictionNum());
	if (!Next.IsEmpty())
	{
		HandlePrediction(Next);
	}
}

// Fires when surrounder resets, on round end or game restart
void UPasPredictionComponent::OnResetSurrounder_Implementation()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BeforeFirstWallSpawnTimer);
	}
}

// Fires once when surrounder initialises for current round
void UPasPredictionComponent::OnInitSurrounder_Implementation()
{
	BindBeforeFirstWallSpawn();
}

// Fires once just before first wall is spawned
void UPasPredictionComponent::OnBeforeFirstWallSpawn_Implementation()
{
	if (!Surrounder)
	{
		// Sibling surrounder vanished, nothing to predict from
		return;
	}
	// Seed cell is where first wall will spawn, prepend it so callers covering more than 1 step get contiguous range
	const FPasCellDataOnSide MyCellData = Surrounder->GetCurrentCellData();
	const int32 PredictionNum = GetPredictionNum();
	TArray<FPasCellDataOnSide> PredictedCells;
	PredictedCells.Reserve(PredictionNum);
	PredictedCells.Add(MyCellData);
	if (PredictionNum > 1)
	{
		PredictedCells.Append(UPasBlueprintFunctionLibrary::GetNextCells(MyCellData, PredictionNum - 1));
	}
	HandlePrediction(PredictedCells);
}

// Schedules callback that fires before first wall spawn
void UPasPredictionComponent::BindBeforeFirstWallSpawn()
{
	const UPasDataAsset& Data = UPasDataAsset::Get();
	const UWorld* World = GetWorld();
	if (!World || Data.WaitBeforeSurroundTime <= 0.f)
	{
		// Data asset missing, world gone, or pre-round wait disabled, nothing to arm
		return;
	}
	// Fire just before first wall step so visualizer has time to warn
	static constexpr float MinPreSpawnInterval = 0.01f;
	const float WaitDelta = FMath::Max(Data.WaitBeforeSurroundTime - Data.WallStepTime, MinPreSpawnInterval);
	World->GetTimerManager().SetTimer(BeforeFirstWallSpawnTimer,
	    FTimerDelegate::CreateUObject(this, &UPasPredictionComponent::OnBeforeFirstWallSpawn),
	    WaitDelta, /*bLoop*/ false);
}

void UPasPredictionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* MyOwner = GetOwner();
	GeneratedMap = Cast<ABmrGeneratedMap>(MyOwner);
	Surrounder = MyOwner ? MyOwner->FindComponentByClass<UPasSurrounderLogicComponent>() : nullptr;
	if (Surrounder)
	{
		Surrounder->OnWallSpawned.AddDynamic(this, &UPasPredictionComponent::OnWallSpawned);
		Surrounder->OnResetSurrounder.AddDynamic(this, &UPasPredictionComponent::OnResetSurrounder);
		Surrounder->OnInitSurrounder.AddDynamic(this, &UPasPredictionComponent::OnInitSurrounder);
	}

	if (Surrounder && Surrounder->IsInitialized())
	{
		OnInitSurrounder();
	}
}

void UPasPredictionComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BeforeFirstWallSpawnTimer);
	}

	if (Surrounder)
	{
		Surrounder->OnWallSpawned.RemoveDynamic(this, &UPasPredictionComponent::OnWallSpawned);
		Surrounder->OnResetSurrounder.RemoveDynamic(this, &UPasPredictionComponent::OnResetSurrounder);
		Surrounder->OnInitSurrounder.RemoveDynamic(this, &UPasPredictionComponent::OnInitSurrounder);
	}

	Super::EndPlay(EndPlayReason);
}
