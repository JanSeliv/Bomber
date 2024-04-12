// Copyright (c) Yevhenii Selivanov

#pragma once

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
 * Internal structure to handle character ready event.
 * Any of public methods can be called in any order.
 */
struct BOMBER_API FOnCharactersReadyHandler
{
public:
	/** Should be called when character is possessed. */
	void OnCharacterPossessed(APlayerCharacter& Character);

	/** Should be called when character ID is assigned or replicated. */
	void OnCharacterIdAssigned(APlayerCharacter& Character);

	/** Should be called when player state is replicated. */
	void OnPlayerStateInit(const AMyPlayerState& PlayerState);

	/** Returns true if the character is ready at this moment. */
	bool IsCharacterReady(const APlayerCharacter* Character) const;

	/** Perform cleanup. */
	void Reset();

private:
	/** Internal data struct to handle character ready event. */
	struct FOnCharacterReadyData
	{
		TWeakObjectPtr<APlayerCharacter> Character = nullptr;
		TWeakObjectPtr<const AMyPlayerState> PlayerState = nullptr;
		bool bIsPossessed = false;
		bool bHasCharacterID = false;
	};

	/** All registered character ready handles. */
	TArray<FOnCharacterReadyData> OnCharacterReadyHandles;

	FOnCharacterReadyData& FindOrAdd(APlayerCharacter& Character);

	/** Broadcasts OnCharacterReady event if all conditions are met. */
	void TryBroadcastOnCharacterReady(APlayerCharacter& Character);
};