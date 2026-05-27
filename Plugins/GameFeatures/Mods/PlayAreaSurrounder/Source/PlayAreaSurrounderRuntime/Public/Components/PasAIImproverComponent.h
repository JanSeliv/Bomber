// Copyright (c) Yevhenii Selivanov

#pragma once

#include "PasPredictionComponent.h"

// Bomber
#include "Structures/BmrCell.h"

#include "PasAIImproverComponent.generated.h"

/**
 * Server-side helper that lifts next predicted cells out of surrounder and exposes them to bots so they avoid upcoming wall positions.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PLAYAREASURROUNDERRUNTIME_API UPasAIImproverComponent final : public UPasPredictionComponent
{
	GENERATED_BODY()

public:
	UPasAIImproverComponent();

	//~ UPasPredictionComponent
	/** When surrounder queries how many cells ahead to predict for this component. */
	virtual int32 GetPredictionNum_Implementation() const override;
	/** When surrounder emits next predicted cells after each wall step. */
	virtual void HandlePrediction_Implementation(const TArray<FPasCellDataOnSide>& PredictedCells) override;
	//~ End UPasPredictionComponent

	/** Returns most recent set of cells AI should treat as dangerous. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	const TSet<FBmrCell>& GetDangerousCells() const { return DangerousCells; }

protected:
	/** Cells AI should currently steer away from. Updated each wall step. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Play Area Surrounder]")
	TSet<FBmrCell> DangerousCells;
};
