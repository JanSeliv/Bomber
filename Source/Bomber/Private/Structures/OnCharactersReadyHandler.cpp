// Copyright (c) Yevhenii Selivanov

#include "Structures/OnCharactersReadyHandler.h"
//---
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"

// Should be called when character is possessed
void FOnCharactersReadyHandler::Broadcast_OnCharacterPossessed(APlayerCharacter& Character)
{
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !IsCharacterReady(&Character)) // Skip if already ready
	{
		FindOrAdd(Character).bIsPossessed = true;
		TryBroadcastOnReady_Internal(Character);
	}
}

// Should be called when player state is replicated
void FOnCharactersReadyHandler::Broadcast_OnPlayerStateInit(const AMyPlayerState& PlayerState)
{
	if (!UUtilsLibrary::HasWorldBegunPlay())
	{
		return;
	}

	APlayerCharacter* Character = PlayerState.GetPawn<APlayerCharacter>();
	if (ensureAlwaysMsgf(Character, TEXT("ERROR: [%i] %hs:\n'Character' is not assigned for next player state: %s"), __LINE__, __FUNCTION__, *PlayerState.GetName())
	    && !IsCharacterReady(Character)) // Skip if already ready
	{
		FindOrAdd(*Character).PlayerState = &PlayerState;
		TryBroadcastOnReady_Internal(*Character);
	}
}

// Returns true if the character is ready at this moment
bool FOnCharactersReadyHandler::IsCharacterReady(const APlayerCharacter* Character) const
{
	if (!Character)
	{
		return false;
	}

	const FOnCharacterReadyData* FoundHandle = OnCharacterReadyHandles.FindByPredicate([&Character](const FOnCharacterReadyData& It)
	{
		return It.Character == Character;
	});

	// Check general conditions
	if (!FoundHandle
	    || !FoundHandle->Character.IsValid()
	    || !FoundHandle->PlayerState.IsValid())
	{
		return false;
	}

	if (!FoundHandle->Character->HasAuthority()
	    && FoundHandle->Character->IsBotControlled())
	{
		// It's a client bot, where controller does not exist by design, so it's ready
		return true;
	}

	// Finally, check if controller is possessed, so character is completely ready
	return FoundHandle->bIsPossessed;
}

// Returns true if the player state is ready at this moment
bool FOnCharactersReadyHandler::IsCharacterReady(const AMyPlayerState* PlayerState) const
{
	return PlayerState && IsCharacterReady(PlayerState->GetPawn<APlayerCharacter>());
}

// Broadcasts OnCharacterReady event if all conditions are met
void FOnCharactersReadyHandler::TryBroadcastOnReady_Internal(APlayerCharacter& Character)
{
	if (!IsCharacterReady(&Character))
	{
		// Not ready yet
		return;
	}

	const UGlobalEventsSubsystem& EventsSubsystem = UGlobalEventsSubsystem::Get();
	AMyPlayerState* PlayerState = CastChecked<AMyPlayerState>(Character.GetPlayerState());

	const int32 CharacterID = PlayerState->GetPlayerId();
	const bool bIsLocalPlayer = PlayerState->IsPlayerStateLocallyControlled();

	if (EventsSubsystem.BP_OnCharacterReady.IsBound())
	{
		EventsSubsystem.BP_OnCharacterReady.Broadcast(&Character, CharacterID);
	}

	if (bIsLocalPlayer && EventsSubsystem.BP_OnLocalCharacterReady.IsBound())
	{
		EventsSubsystem.BP_OnLocalCharacterReady.Broadcast(&Character, CharacterID);
	}

	if (EventsSubsystem.BP_OnPlayerStateReady.IsBound())
	{
		EventsSubsystem.BP_OnPlayerStateReady.Broadcast(PlayerState, CharacterID);
	}

	if (bIsLocalPlayer && EventsSubsystem.BP_OnLocalPlayerStateReady.IsBound())
	{
		EventsSubsystem.BP_OnLocalPlayerStateReady.Broadcast(PlayerState, CharacterID);
	}
}

// Perform cleanup
void FOnCharactersReadyHandler::Reset()
{
	OnCharacterReadyHandles.Empty();
}

FOnCharactersReadyHandler::FOnCharacterReadyData& FOnCharactersReadyHandler::FindOrAdd(APlayerCharacter& Character)
{
	FOnCharacterReadyData* FoundHandle = OnCharacterReadyHandles.FindByPredicate([&Character](const FOnCharacterReadyData& It)
	{
		return It.Character == &Character;
	});

	if (FoundHandle)
	{
		return *FoundHandle;
	}

	FOnCharacterReadyData NewHandle;
	NewHandle.Character = &Character;
	return OnCharacterReadyHandles.Emplace_GetRef(MoveTemp(NewHandle));
}