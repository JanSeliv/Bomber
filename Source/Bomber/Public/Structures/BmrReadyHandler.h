// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "UObject/WeakObjectPtr.h"

/*********************************************************************************************
 * The FOnCharactersReadyHandler broadcasts the readiness of player characters in multiplayer
 * settings once their possession, player state initialization, and ID assignment are complete.
 * These components can be initialized in any order, and the Handler efficiently tracks and
 * synchronizes their status to ensure accurate and timely readiness notifications for gameplay.
 ********************************************************************************************* */

class APlayerCharacter;
class AMyPlayerState;

/**
 * Encapsulates the managements of 'On Player Ready' delegates from 'GlobalEventsSubsystem'.
 */
struct BOMBER_API FOnCharactersReadyHandler
{
public:
	/*********************************************************************************************
	 * Public Broadcasters.
	 * Once all are broadcasted, original delegate will be automatically called.
	 * Can be called in any order.
	 ********************************************************************************************* */
public:
	/** Should be called when character is possessed. */
	void Broadcast_OnCharacterPossessed(APlayerCharacter& Character);

	/** Should be called when player state is replicated. */
	void Broadcast_OnPlayerStateInit(const AMyPlayerState& PlayerState);

	/** Should be called when character is added to the Generated Map. */
	void Broadcast_OnCharacterAdded(APlayerCharacter& Character);

	/*********************************************************************************************
	 * Public Helpers
	 ********************************************************************************************* */
public:
	/** Returns true if the character is ready at this moment. */
	bool IsCharacterReady(const APlayerCharacter* Character) const;

	/** Returns true if the player state is ready at this moment. */
	bool IsCharacterReady(const AMyPlayerState* PlayerState) const;

	/** Perform cleanup. */
	void Reset();

	/*********************************************************************************************
	 * Internal handling
	 ********************************************************************************************* */
private:
	/** Internal data struct to handle character ready event. */
	struct FOnCharacterReadyData
	{
		TWeakObjectPtr<APlayerCharacter> Character = nullptr;
		TWeakObjectPtr<const AMyPlayerState> PlayerState = nullptr;
		bool bIsPossessed = false;
		bool bIsAddedOnGeneratedMap = false;
	};

	/** All registered character ready handles. */
	TArray<FOnCharacterReadyData> OnCharacterReadyHandles;

	FOnCharacterReadyData& FindOrAdd(APlayerCharacter& Character);

	/** Returns true if player controller is possessed and ready at this moment.*
	 * It does not check any other conditions, only possession. */
	static bool IsCharacterPossessed(const FOnCharacterReadyData& FoundHandle);

	/** Is internal method, shouldn't be called directly, instead Broadcast_ methods should be used. */
	void TryBroadcastOnReady_Internal(APlayerCharacter& Character);
};
