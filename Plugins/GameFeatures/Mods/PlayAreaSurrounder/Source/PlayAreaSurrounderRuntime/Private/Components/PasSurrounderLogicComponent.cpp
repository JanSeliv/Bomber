// Copyright (c) Yevhenii Selivanov

#include "Components/PasSurrounderLogicComponent.h"

// PAS
#include "Data/PasDataAsset.h"
#include "Data/PasNextCellOutput.h"
#include "PasBlueprintFunctionLibrary.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h" // EBmrActorType
#include "DalSubsystem.h"
#include "Structures/BmrCell.h"
#include "Structures/BmrGameStateTag.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h" // FGameplayEventData
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Structures/BmrGameplayTags.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PasSurrounderLogicComponent)

UPasSurrounderLogicComponent::UPasSurrounderLogicComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Number of sides surrounder has passed so far
int32 UPasSurrounderLogicComponent::GetPassedCellsNum() const
{
	return MyCellData.Hr + MyCellData.Vd + MyCellData.Hl + MyCellData.Vu;
}

void UPasSurrounderLogicComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* MyOwner = GetOwner();
	GeneratedMap = Cast<ABmrGeneratedMap>(MyOwner);
	if (!MyOwner || !MyOwner->HasAuthority())
	{
		return;
	}

	UDalSubsystem::Get().ListenForDataAsset<UPasDataAsset>(this, &ThisClass::OnDataAssetLoaded);
}

// Fires once PlayAreaSurrounder data asset finishes async load via UDalSubsystem
void UPasSurrounderLogicComponent::OnDataAssetLoaded_Implementation(const UPasDataAsset* DataAsset)
{
	if (const AActor* MyOwner = GetOwner(); !MyOwner || !MyOwner->HasAuthority())
	{
		// Clients have data asset cached too but state machine is server-only
		return;
	}

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &UPasSurrounderLogicComponent::OnGameStateChanged);
}

// Fires when "Event.GameState.Changed" global message arrives
void UPasSurrounderLogicComponent::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (const AActor* MyOwner = GetOwner(); !MyOwner || !MyOwner->HasAuthority())
	{
		// Client-side, state machine is server-only
		return;
	}

	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::InGame))
	{
		StartSurrounderForRound();
	}
	else if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaitBeforeSurroundTimer);
		World->GetTimerManager().ClearTimer(WallStepTimer);
		MyCellData = FPasCellDataOnSide::EmptyData;
		bIsInitialized = false;
		OnResetSurrounder.Broadcast();
	}
}

// Server-only entry point: starts surrounder cycle for current round
void UPasSurrounderLogicComponent::StartSurrounderForRound()
{
	const UPasDataAsset& Data = UPasDataAsset::Get();
	const UWorld* World = GetWorld();
	if (!World)
	{
		// World gone, cannot arm timers
		return;
	}

	// Pick starting side: when data asset
	// opts in, pick random from 4 sides, otherwise begin at HorizontalRight
	static const TArray<EPasTurnType> StartSides = {
	    EPasTurnType::HorizontalRight, EPasTurnType::VerticalDown,
	    EPasTurnType::HorizontalLeft, EPasTurnType::VerticalUp};
	MyCellData = FPasCellDataOnSide::EmptyData;
	MyCellData.TurnType = Data.bRandomlySelectTurnType
	                          ? StartSides[FMath::RandRange(0, StartSides.Num() - 1)]
	                          : EPasTurnType::HorizontalRight;

	// Seed MyCellData at first cell of chosen side
	if (const FBmrCell Start = UPasBlueprintFunctionLibrary::GetFirstCellOnSide(MyCellData.TurnType); Start.IsValid())
	{
		int32 Col = 0;
		int32 Row = 0;
		UBmrCellUtilsLibrary::GetPositionByCellOnLevel(Start, Col, Row);
		MyCellData.Row = Row;
		MyCellData.Column = Col;
	}

	bIsInitialized = true;
	OnInitSurrounder.Broadcast();

	// Arm one-shot timer for
	// WaitBeforeSurroundTime, then OnFinishedWaitBeforeSurroundTime spawns
	// first wall and arms recurring WallStepTimer
	static constexpr float MinTimerInterval = 0.01f;
	const float ArmDelay = FMath::Max(Data.WaitBeforeSurroundTime, MinTimerInterval);
	World->GetTimerManager().SetTimer(WaitBeforeSurroundTimer,
	    FTimerDelegate::CreateUObject(this, &UPasSurrounderLogicComponent::TickPreRoundWait),
	    ArmDelay, /*bLoop*/ false);
}

// Fires once pre-round wait elapses, spawns first wall and begins recurring wall-step cycle
void UPasSurrounderLogicComponent::TickPreRoundWait()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		// Tick fired after world teardown, abandon timer setup
		return;
	}
	// BmrGeneratedMap generates its cell grid asynchronously. Until grid
	// populates, GetLastRow/Col returns -1 and GetNextCell would short-circuit
	// on first step. Self-reschedule until grid is ready, then seed MyCellData
	// from loaded grid
	if (UBmrCellUtilsLibrary::GetLastRowIndexOnLevel() < 0
	    || UBmrCellUtilsLibrary::GetLastColumnIndexOnLevel() < 0)
	{
		// Grid not generated yet, retry in 1s
		static constexpr float GridRetryInterval = 1.0f;
		World->GetTimerManager().SetTimer(WaitBeforeSurroundTimer,
		    FTimerDelegate::CreateUObject(this, &UPasSurrounderLogicComponent::TickPreRoundWait),
		    GridRetryInterval, /*bLoop*/ false);
		return;
	}
	if (const FBmrCell ReseededStart = UPasBlueprintFunctionLibrary::GetFirstCellOnSide(MyCellData.TurnType); ReseededStart.IsValid())
	{
		int32 Col = 0;
		int32 Row = 0;
		UBmrCellUtilsLibrary::GetPositionByCellOnLevel(ReseededStart, Col, Row);
		MyCellData.Row = Row;
		MyCellData.Column = Col;
	}
	// Spawn FIRST wall at seed
	// cell (no advance), then arm loop. TickWallStep advances first, so we
	// call SpawnWallOnCurrentCell directly here to avoid skipping seed cell
	SpawnWallOnCurrentCell();
	ArmOrRearmWallStepTimer();
}

// Arm or re-arm recurring wall-step cadence using current passed-sides count
void UPasSurrounderLogicComponent::ArmOrRearmWallStepTimer()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		// World gone, cannot drive timer
		return;
	}
	const float Step = UPasBlueprintFunctionLibrary::CalcWallStepTime(GetPassedCellsNum());
	static constexpr float MinTimerInterval = 0.01f;
	const float StepInterval = FMath::Max(Step, MinTimerInterval);
	World->GetTimerManager().ClearTimer(WallStepTimer);
	World->GetTimerManager().SetTimer(WallStepTimer,
	    FTimerDelegate::CreateUObject(this, &UPasSurrounderLogicComponent::TickWallStep),
	    StepInterval, /*bLoop*/ true);
}

// Fires every wall-step interval while surrounder is spiralling inward
void UPasSurrounderLogicComponent::TickWallStep()
{
	// Advance MyCellData FIRST, then spawn
	const FPasNextCellOutput Next = UPasBlueprintFunctionLibrary::GetNextCell(MyCellData);
	const EPasTurnType OldSide = MyCellData.TurnType;
	MyCellData = Next.CellDataOnSurrounderSide;

	if (Next.bIsOutsideBoundaries)
	{
		if (const UWorld* StepWorld = GetWorld())
		{
			StepWorld->GetTimerManager().ClearTimer(WallStepTimer);
		}
		OnResetSurrounder.Broadcast();
		return;
	}

	// Spawn at now-advanced MyCellData, broadcast happens BEFORE spawn so
	// receivers see MyCellData as spawn target (their GetNextCells then
	// returns upcoming next-iteration target, not next-of-next)
	SpawnWallOnCurrentCell();

	if (MyCellData.TurnType != OldSide)
	{
		OnSideChanged.Broadcast(GetPassedCellsNum());
		// Post-spawn: rearm with CalcWallStepTime
		// so cadence speeds up by WallStepTimeMultiplierForEachSide
		ArmOrRearmWallStepTimer();
	}
}

// Spawns wall at current cell on server, replaces any prior actor on same cell
void UPasSurrounderLogicComponent::SpawnWallOnCurrentCell()
{
	if (!GeneratedMap)
	{
		// No map, nothing to spawn into
		return;
	}
	const FBmrCell Cell = UBmrCellUtilsLibrary::GetCellByPositionOnLevel(MyCellData.Column, MyCellData.Row);
	if (!Cell.IsValid())
	{
		// Cell off grid, abort step, TickWallStep keeps stepping
		return;
	}
	// Broadcast BEFORE spawning so listeners see MyCellData = spawn target
	// GetNextCells(MyCellData, N) for visualizer then returns upcoming
	// step (not next-of-next)
	OnWallSpawned.Broadcast();

	if (!UBmrCellUtilsLibrary::IsEmptyCellWithoutActor(Cell))
	{
		// Destroy whatever sits on cell (perimeter wall, box, power-up) so surrounder wall takes precedence
		GeneratedMap->DestroyLevelActorsOnCells(TSet<FBmrCell>{Cell});
	}
	GeneratedMap->K2_SpawnActorByType(EBmrActorType::Wall, Cell);
}

void UPasSurrounderLogicComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WallStepTimer);
		World->GetTimerManager().ClearTimer(WaitBeforeSurroundTimer);
	}

	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	Super::EndPlay(EndPlayReason);
}
