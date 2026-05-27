// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

#include "PasPredictionComponent.generated.h"

/**
 * Abstract base shared by visualizer and AI improver. Caches owning play-area
 * actor and sibling surrounder, mirrors surrounder lifecycle delegates, and
 * exposes prediction entry points for subclasses to override.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PLAYAREASURROUNDERRUNTIME_API UPasPredictionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPasPredictionComponent();

	/** Number of cells ahead this predictor should reason about. Override per derived component. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]")
	int32 GetPredictionNum() const;
	virtual int32 GetPredictionNum_Implementation() const { return 1; }

	/** Fires once per wall spawn with next predicted cells. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]")
	void HandlePrediction(const TArray<struct FPasCellDataOnSide>& PredictedCells);
	virtual void HandlePrediction_Implementation(const TArray<FPasCellDataOnSide>& PredictedCells) { }

	/** Fires once just before first wall is spawned. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]")
	void OnBeforeFirstWallSpawn();
	virtual void OnBeforeFirstWallSpawn_Implementation();

protected:
	/** Cached sibling surrounder logic component on same actor. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	TObjectPtr<class UPasSurrounderLogicComponent> Surrounder = nullptr;

	/** Cached owning play-area actor. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	TObjectPtr<class ABmrGeneratedMap> GeneratedMap = nullptr;

	/** Timer that fires shortly before first wall step. Non-BP: FTimerHandle. */
	FTimerHandle BeforeFirstWallSpawnTimer;

	/** Fires after each wall step on sibling surrounder. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]")
	void OnWallSpawned();
	virtual void OnWallSpawned_Implementation();

	/** Fires when surrounder resets (round end or game restart). */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]")
	void OnResetSurrounder();
	virtual void OnResetSurrounder_Implementation();

	/** Fires once when surrounder initialises for current round. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]")
	void OnInitSurrounder();
	virtual void OnInitSurrounder_Implementation();

	/** Schedules callback that fires before first wall spawn. */
	void BindBeforeFirstWallSpawn();

	//~ UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent
};
