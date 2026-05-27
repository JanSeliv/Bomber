// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

// PAS
#include "Data/PasCellDataOnSide.h"

#include "PasSurrounderLogicComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPasOnWallSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPasOnResetSurrounder);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPasOnInitSurrounder);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPasOnSideChanged, int32, PassedSidesNum);

/**
 * Server-only surrounder state machine: arms pre-round wait, then steps
 * around play area spawning walls and broadcasting wall-spawn, side-change,
 * and reset events to sibling observers.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PLAYAREASURROUNDERRUNTIME_API UPasSurrounderLogicComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UPasSurrounderLogicComponent();

	/** Returns current cell data surrounder occupies on its active side. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	const FPasCellDataOnSide& GetCurrentCellData() const { return MyCellData; }

	/** Number of sides surrounder has passed so far. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	int32 GetPassedCellsNum() const;

	/** True once round setup completed, so late-binding sibling components can detect and replay initialisation. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	bool IsInitialized() const { return bIsInitialized; }

	/** Fires after each wall spawn on server, mirrored to local subscribers on same actor. */
	UPROPERTY(BlueprintAssignable, Category = "[Play Area Surrounder]")
	FPasOnWallSpawned OnWallSpawned;

	/** Fires when surrounder resets, on round end or game restart. */
	UPROPERTY(BlueprintAssignable, Category = "[Play Area Surrounder]")
	FPasOnResetSurrounder OnResetSurrounder;

	/** Fires once when surrounder initialises for current round. */
	UPROPERTY(BlueprintAssignable, Category = "[Play Area Surrounder]")
	FPasOnInitSurrounder OnInitSurrounder;

	/** Fires when surrounder transitions to new side, turn boundary. */
	UPROPERTY(BlueprintAssignable, Category = "[Play Area Surrounder]")
	FPasOnSideChanged OnSideChanged;

protected:
	/** Cached owning play-area actor. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	TObjectPtr<class ABmrGeneratedMap> GeneratedMap = nullptr;

	/** Current surrounder cell + side state. Server-authoritative. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	FPasCellDataOnSide MyCellData = FPasCellDataOnSide::EmptyData;

	/** Server-side flag set after round init broadcast, gated for late-binding sibling replay. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	bool bIsInitialized = false;

	/** Recurring per-step timer that drives wall spawning. Non-BP: FTimerHandle. */
	FTimerHandle WallStepTimer;

	/** One-shot pre-round wait timer. Non-BP: FTimerHandle. */
	FTimerHandle WaitBeforeSurroundTimer;

	/** Fires when "Event.GameState.Changed" global message arrives. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Fires once PlayAreaSurrounder data asset finishes async load via UDalSubsystem. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]", meta = (BlueprintProtected))
	void OnDataAssetLoaded(const class UPasDataAsset* DataAsset);

	/** Server-only entry point: starts surrounder cycle for current round. */
	void StartSurrounderForRound();

	/** Fires once pre-round wait elapses, spawns first wall and begins recurring wall-step cycle. */
	void TickPreRoundWait();

	/** Fires every wall-step interval while surrounder is spiralling inward. */
	void TickWallStep();

	/** Spawns wall at current cell on server, replaces any prior actor on same cell. */
	void SpawnWallOnCurrentCell();

	/** Arm or re-arm recurring wall-step cadence using current passed-sides count. */
	void ArmOrRearmWallStepTimer();

	//~ UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent
};
