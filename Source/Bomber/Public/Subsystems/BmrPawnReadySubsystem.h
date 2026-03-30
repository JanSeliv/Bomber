// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "BmrPawnReadySubsystem.generated.h"

/*********************************************************************************************
 * The UBmrPawnReadySubsystem broadcasts the readiness of pawns and player states in multiplayer
 * once their possession, player state initialization, and ID assignment are complete.
 * These components can be initialized in any order, and the Handler efficiently tracks and
 * synchronizes their status to ensure accurate and timely readiness notifications for gameplay.
 ********************************************************************************************* */

class ABmrPawn;
class ABmrPlayerState;

/**
 * Encapsulates the managements of 'On Player Ready' delegates from 'GlobalEventsSubsystem'.
 */
UCLASS()
class BOMBER_API UBmrPawnReadySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns the Pawn Ready Subsystem, is checked and will crash if can't be obtained */
	static UBmrPawnReadySubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the pointer to this Subsystem, nullptr if world is not available */
	static UBmrPawnReadySubsystem* GetPawnReadySubsystem(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Public Broadcasters.
	 * Once all are broadcasted, original delegate will be automatically called.
	 * Can be called in any order.
	 ********************************************************************************************* */
public:
	/** Should be called when pawn is possessed. */
	void Broadcast_OnPawnPossessed(ABmrPawn& Pawn);

	/** Should be called when player state is replicated. */
	void Broadcast_OnPlayerStateInit(const ABmrPlayerState& PlayerState);

	/** Should be called when pawn is added to the Generated Map. */
	void Broadcast_OnPawnAdded(ABmrPawn& Pawn);

	/*********************************************************************************************
	 * Public Helpers
	 ********************************************************************************************* */
public:
	/** Returns true if the pawn is ready at this moment. */
	bool IsReady(const ABmrPawn* Pawn) const;

	/** Returns true if the player state is ready at this moment. */
	bool IsReady(const ABmrPlayerState* PlayerState) const;

	/** Perform cleanup. */
	void Reset();

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Clears ready handler data */
	virtual void Deinitialize() override;

	/*********************************************************************************************
	 * Internal handling
	 ********************************************************************************************* */
private:
	/** Internal data struct to handle pawn ready event. */
	struct FOnReadyData
	{
		TWeakObjectPtr<ABmrPawn> Pawn = nullptr;
		TWeakObjectPtr<const ABmrPlayerState> PlayerState = nullptr;
		bool bIsPossessed = false;
		bool bIsAddedOnGeneratedMap = false;
	};

	/** All registered pawn ready handles. */
	TArray<FOnReadyData> OnReadyHandles;

	FOnReadyData& FindOrAdd(ABmrPawn& Pawn);

	/** Returns true if player controller is possessed and ready at this moment.*
	 * It does not check any other conditions, only possession. */
	static bool IsPawnPossessed(const FOnReadyData& FoundHandle);

	/** Is internal method, shouldn't be called directly, instead Broadcast_ methods should be used. */
	void TryBroadcastOnReady_Internal(ABmrPawn& Pawn);
};
