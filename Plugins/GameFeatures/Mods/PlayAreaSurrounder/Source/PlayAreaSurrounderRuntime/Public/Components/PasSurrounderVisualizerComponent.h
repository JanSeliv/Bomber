// Copyright (c) Yevhenii Selivanov

#pragma once

#include "PasPredictionComponent.h"

#include "PasSurrounderVisualizerComponent.generated.h"

/**
 * Owns runtime-spawned RectLight that warns about next wall cell.
 * Replicates target cell location and flicker interval so remote clients mirror server.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PLAYAREASURROUNDERRUNTIME_API UPasSurrounderVisualizerComponent final : public UPasPredictionComponent
{
	GENERATED_BODY()

public:
	UPasSurrounderVisualizerComponent();

	//~ UPasPredictionComponent
	/** When surrounder queries prediction depth for this component. */
	virtual int32 GetPredictionNum_Implementation() const override { return 1; }
	/** When surrounder emits next predicted cells after each wall step. */
	virtual void HandlePrediction_Implementation(const TArray<FPasCellDataOnSide>& PredictedCells) override;
	/** When pre-spawn callback fires before first wall. */
	virtual void OnBeforeFirstWallSpawn_Implementation() override;
	//~ End UPasPredictionComponent

protected:
	/** Runtime-spawned RectLight that visualises next surrounded cell.*/
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	TObjectPtr<class URectLightComponent> LightComponent = nullptr;

	/** Server-replicated world location of next surround target.
	 * @TODO JanSeliv Vqol05dU - remove both OnRep_ properties: each client can directly update light from local prediction. */
	UPROPERTY(ReplicatedUsing = OnRep_VisualizerLocation, BlueprintReadOnly, Category = "[Play Area Surrounder]")
	FVector VisualizerLocation = FVector::ZeroVector;

	/** Server-replicated per-flicker interval. */
	UPROPERTY(ReplicatedUsing = OnRep_FlickerTime, BlueprintReadOnly, Category = "[Play Area Surrounder]")
	double FlickerTime = 0.0;

	/** Active flicker timer, local. Non-BP: FTimerHandle. */
	FTimerHandle FlickerTimer;

	/** Authority-only: compute new replicated location from next cell and apply locally. */
	UFUNCTION(BlueprintCallable, Category = "[Play Area Surrounder]")
	void Authority_MoveVisualizer(const struct FBmrCell& OnCell);

	/** Builds (or rebuilds) RectLight and reapplies replicated state. */
	UFUNCTION(BlueprintCallable, Category = "[Play Area Surrounder]")
	void CreateVisualiser();

	/** Fires once PlayAreaSurrounder data asset finishes async load via UDalSubsystem. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Play Area Surrounder]", meta = (BlueprintProtected))
	void OnDataAssetLoaded(const class UPasDataAsset* DataAsset);

	/** Pushes replicated target location onto runtime light, null-safe when light not yet built. */
	UFUNCTION(BlueprintCallable, Category = "[Play Area Surrounder]")
	void UpdateVisualizerLocation_Internal();

	/** (Re)arms flicker timer using current replicated interval, null-safe when light not yet built. */
	UFUNCTION(BlueprintCallable, Category = "[Play Area Surrounder]")
	void TriggerFlickerTimer();

	/** Fires every flicker interval while surrounder is active. */
	void FlickerTick();

	/** Fires when surrounder transitions to next side. */
	UFUNCTION(BlueprintCallable, Category = "[Play Area Surrounder]")
	void HandleSurrounderSideChanged(int32 PassedSidesNum);

	/** Fires when server replicates updated VisualizerLocation to client.
	 *  BP-expose exception: engine RepNotify wiring requires bare UFUNCTION, BlueprintNativeEvent breaks ReplicatedUsing dispatch. */
	UFUNCTION()
	void OnRep_VisualizerLocation();

	/** Fires when server replicates updated FlickerTime to client.
	 *  BP-expose exception: engine RepNotify wiring requires bare UFUNCTION. */
	UFUNCTION()
	void OnRep_FlickerTime();

	//~ UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UActorComponent

	//~ UPasPredictionComponent
	/** When surrounder resets, on round end or game restart. */
	virtual void OnResetSurrounder_Implementation() override;
	//~ End UPasPredictionComponent
};
