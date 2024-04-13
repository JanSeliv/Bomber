// Copyright (c) Yevhenii Selivanov

#include "Structures/OnCharactersReadyHandler.h"
//---
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"

// Should be called when character is possessed
void FOnCharactersReadyHandler::OnCharacterPossessed(APlayerCharacter& Character)
{
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !IsCharacterReady(&Character)) // Skip if already ready
	{
		FindOrAdd(Character).bIsPossessed = true;
		TryBroadcastOnCharacterReady(Character);
	}
}

// Should be called when character ID is assigned or replicated
void FOnCharactersReadyHandler::OnCharacterIdAssigned(APlayerCharacter& Character)
{
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !IsCharacterReady(&Character)) // Skip if already ready
	{
		ensureAlwaysMsgf(Character.GetCharacterID() != INDEX_NONE, TEXT("ERROR: [%i] %hs:\n'Character ID' is not assigned for next character: %s"), __LINE__, __FUNCTION__, *Character.GetName());
		FindOrAdd(Character).bHasCharacterID = true;
		TryBroadcastOnCharacterReady(Character);
	}
}

// Should be called when player state is replicated
void FOnCharactersReadyHandler::OnPlayerStateInit(const AMyPlayerState& PlayerState)
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
		TryBroadcastOnCharacterReady(*Character);
	}
}

// Returns true if the character is ready at this moment
bool FOnCharactersReadyHandler::IsCharacterReady(const APlayerCharacter* Character) const
{
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

// Broadcasts OnCharacterReady event if all conditions are met
void FOnCharactersReadyHandler::TryBroadcastOnCharacterReady(APlayerCharacter& Character)
{
	if (IsCharacterReady(&Character))
	{
		UGlobalEventsSubsystem::Get().OnCharacterReady.Broadcast(&Character, Character.GetCharacterID());
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