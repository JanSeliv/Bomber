// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/StaticMeshComponent.h"

#include "BmrPlayerArrowStartComponent.generated.h"

/**
 * Static mesh component displaying arrow above the local player during match start
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UBmrPlayerArrowStartComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UBmrPlayerArrowStartComponent();

	/** Controls arrow visibility, tick, and animation state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetArrowEnabled(bool bShouldEnable);

	/** Returns true if arrow can be enabled for the local player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool CanEnableArrow() const;

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Curve table defining arrow bounce animation over time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UCurveTable> AnimationCurveTable = nullptr;

	/** Accumulated time since animation start. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	float StartTime = 0.f;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Updates animation from curve table. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Manages component state based on game state and local control. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when pawn controller changes. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
};