// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MoverComponent.h"

#include "BmrMoverComponent.generated.h"

enum class EBmrCurrentGameState : uint8;

/*
 * Replaces the Character Movement Component (CMR) for next purposes:
 * - Better local movement prediction causing less jittering.
 * - Modular movement modes that can be easily extended or injected.
 * It caches gameplay-related data (such as inputs, states etc) and provides to other systems such as movement modes.
 * All the settings are set in Details Panel of pawn blueprint due to instanced properties.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UBmrMoverComponent : public UMoverComponent
{
	GENERATED_BODY()

public:
	/** Moves owner in given direction.
	 * Is called by both player (from input) and AI.
	 * @param Direction - Normalized direction to move in the world space, can be zero to stop moving. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void RequestMoveByIntent(const FVector& Direction);

	/** When blocked, all movement inputs are ignored and the owner pawn will not move.
	 * Alternatively, UBmrPlayerDataAsset::BlockMovementEffect can be applied/removed directly. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetBlockMovement(bool bShouldBlock);

	/** Returns true if move inputs are currently ignored and the owner pawn can not move.
	 * Alternatively, BmrGameplayTags::GameplayEffect::Block::Movement tag can be checked directly. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	bool IsBlockedMovement() const;

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Cached move input for the current simulation frame.
	 * This is used to accumulate user input or AI state into a single vector that can be processed by the movement system. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	FVector CurrentMoveInput = FVector::ZeroVector;

	/** Cached skate power-up attribute value.
	 * Is used in walking mode to increase the movement speed when player picked up a Skate item. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	float CachedSkatePowerupAttribute = 0.f;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Consumes cached inputs to be processed by other systems such as movement modes. */
	virtual void ProduceInput(const int32 DeltaTimeMS, FMoverInputCmdContext* Cmd) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called when this character is ready to be used. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPawnReady(class ABmrPawn* Pawn, int32 PlayerId);

	/** Listen to react when entered to different game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(EBmrCurrentGameState CurrentGameState);

	/** Called when owner is added on the Generated Map, on both server and client. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnOwnerAddedToLevel(class UBmrMapComponent* MapComponent);

	/** Called when owner is unregistered from the Generated Map, on both server and client. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPreRemovedFromLevel(class UBmrMapComponent* MapComponent, UObject* DestroyCauser);

	/** Event called after a pawn's controller has changed, on the server and owning client. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	/** Broadcast at the end of a simulation tick after movement has occurred, but allowing additions/modifications to the state. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostMove(const FMoverTimeStep& TimeStep, FMoverSyncState& SyncState, FMoverAuxStateContext& AuxState);

	/** Is called by Move Input Action when player pressed the move input button, e.g: WASD or Arrow keys.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnMoveInputTriggered(const struct FInputActionValue& ActionValue);

	/** Is called by Move Input Action when player released the move input button, e.g: WASD or Arrow keys. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnMoveInputCompleted(const struct FInputActionValue& ActionValue);

	/** Is called when the Skate attribute is changed, e.g: when player picked up a Skate item. */
	void OnSkateAttributeChanged(const struct FOnAttributeChangeData& OnAttributeChangeData);
};