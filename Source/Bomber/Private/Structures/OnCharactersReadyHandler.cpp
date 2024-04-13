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

// Should be called when character ID is assigned or replicated
void FOnCharactersReadyHandler::Broadcast_OnCharacterIdAssigned(APlayerCharacter& Character)
{
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !IsCharacterReady(&Character)) // Skip if already ready
	{
		ensureAlwaysMsgf(Character.GetCharacterID() != INDEX_NONE, TEXT("ERROR: [%i] %hs:\n'Character ID' is not assigned for next character: %s"), __LINE__, __FUNCTION__, *Character.GetName());
		FindOrAdd(Character).bHasCharacterID = true;
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

	return FoundHandle
	       && FoundHandle->Character.IsValid()
	       && FoundHandle->PlayerState.IsValid()
	       && (FoundHandle->bIsPossessed || !FoundHandle->Character->HasAuthority()) // Possessed or AI
	       && FoundHandle->bHasCharacterID;
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

	const int32 CharacterID = Character.GetCharacterID();
	const bool bIsLocalPlayer = Character.IsLocallyControlled() && Character.IsPlayerControlled();

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