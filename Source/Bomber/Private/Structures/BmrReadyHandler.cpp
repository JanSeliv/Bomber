// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrReadyHandler.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"

// Should be called when Pawn is possessed
void FBmrReadyHandler::Broadcast_OnPawnPossessed(ABmrPawn& Pawn)
{
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !IsReady(&Pawn)) // Skip if already ready
	{
		FindOrAdd(Pawn).bIsPossessed = true;
		TryBroadcastOnReady_Internal(Pawn);
	}
}

// Should be called when player state is replicated
void FBmrReadyHandler::Broadcast_OnPlayerStateInit(const ABmrPlayerState& PlayerState)
{
	if (!UUtilsLibrary::HasWorldBegunPlay())
	{
		return;
	}

	ABmrPawn* Pawn = PlayerState.GetPawn<ABmrPawn>();
	if (ensureAlwaysMsgf(Pawn, TEXT("ERROR: [%i] %hs:\n'Pawn' is not assigned for next player state: %s"), __LINE__, __FUNCTION__, *PlayerState.GetName())
	    && !IsReady(Pawn)) // Skip if already ready
	{
		FindOrAdd(*Pawn).PlayerState = &PlayerState;
		TryBroadcastOnReady_Internal(*Pawn);
	}
}

// Should be called when Pawn is added to the Generated Map
void FBmrReadyHandler::Broadcast_OnPawnAdded(ABmrPawn& Pawn)
{
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !IsReady(&Pawn)) // Skip if already ready
	{
		FindOrAdd(Pawn).bIsAddedOnGeneratedMap = true;
		TryBroadcastOnReady_Internal(Pawn);
	}
}

// Returns true if the pawn is ready at this moment
bool FBmrReadyHandler::IsReady(const ABmrPawn* Pawn) const
{
	if (!Pawn)
	{
		return false;
	}

	const FOnReadyData* FoundHandle = OnReadyHandles.FindByPredicate([&Pawn](const FOnReadyData& It)
	{
		return It.Pawn == Pawn;
	});

	return FoundHandle
	       && FoundHandle->Pawn.IsValid()
	       && FoundHandle->PlayerState.IsValid()
	       && FoundHandle->bIsAddedOnGeneratedMap
	       && IsPawnPossessed(*FoundHandle);
}

// Returns true if the player state is ready at this moment
bool FBmrReadyHandler::IsReady(const ABmrPlayerState* PlayerState) const
{
	return PlayerState && IsReady(PlayerState->GetPawn<ABmrPawn>());
}

// Returns true if player controller is possessed and ready at this moment
bool FBmrReadyHandler::IsPawnPossessed(const FOnReadyData& FoundHandle)
{
	/*
	| Row | Entity  | Details                         | Authority    | Possession   | Locally Controlled | Result |
	|-----|---------|---------------------------------|--------------|--------------|--------------------|--------|
	| 1   | Player  | Local or Host                   | Has Authority| Possessed    | Yes                | true   |
	| 2   | Bot     | In Host World                   | Has Authority| Possessed    | No                 | true   |
	| 3   | Player  | Local Client (Autonomous Proxy) | No Authority | Possessed    | Yes                | true   |
	| 4   | Bot     | In Client World                 | No Authority | Not Possessed| No                 | true   |
	| 5   | Player  | Local Client (Autonomous Proxy) | No Authority | Not Possessed| Yes                | false  |
	| 6   | Player  | Other Client or Remote Host     | No Authority | Not Possessed| No                 | true   |
	*/

	if (FoundHandle.bIsPossessed)
	{
		return true;
	}

	const bool bHasAuthority = FoundHandle.Pawn->HasAuthority();
	const bool bIsBot = FoundHandle.PlayerState->IsABot();
	const bool bIsLocallyControlled = FoundHandle.Pawn->IsLocallyControlled();

	if (bHasAuthority)
	{
		// Is host, both players and bots that should be possessed
		return false;
	}

	if (bIsBot
	    && !bIsLocallyControlled)
	{
		// Matches Row 4: Bot (In Client World), where controller does not exist by design
		return true;
	}

	if (bIsLocallyControlled
	    && !FoundHandle.bIsPossessed)
	{
		// Matches Row 5: Player - Local Client (Autonomous Proxy) not possessed
		return false;
	}

	if (!bIsBot
	    && !bIsLocallyControlled)
	{
		// Matches Row 6: Player - Other Client or Remote Host
		return true;
	}

	return ensureMsgf(false, TEXT("ASSERT: [%i] %hs:\nUnhandled condition!\nPawn: '%s' | PlayerState: '%s' | bIsPossessed: %s | bIsBot: %s | bIsLocallyControlled: %s | HasAuthority: %s | ID: %i"),
	    __LINE__, __FUNCTION__, *GetNameSafe(FoundHandle.Pawn.Get()), *GetNameSafe(FoundHandle.PlayerState.Get()), FoundHandle.bIsPossessed ? TEXT("true") : TEXT("false"), bIsBot ? TEXT("true") : TEXT("false"), bIsLocallyControlled ? TEXT("true") : TEXT("false"), FoundHandle.Pawn->HasAuthority() ? TEXT("true") : TEXT("false"), FoundHandle.PlayerState->GetPlayerId());
}

// Broadcasts Pawn Ready event if all conditions are met
void FBmrReadyHandler::TryBroadcastOnReady_Internal(ABmrPawn& Pawn)
{
	if (!IsReady(&Pawn))
	{
		// Not ready yet
		return;
	}

	// Broadcast Pawn Ready event via Gameplay Message Router
	FGameplayEventData PawnPayload;
	PawnPayload.EventTag = BmrGameplayTags::Event::Player_PawnReady;
	PawnPayload.Instigator = &Pawn;
	UBmrGameplayMessageSubsystem::BroadcastMessage(PawnPayload);
}

// Perform cleanup
void FBmrReadyHandler::Reset()
{
	OnReadyHandles.Empty();
}

FBmrReadyHandler::FOnReadyData& FBmrReadyHandler::FindOrAdd(ABmrPawn& Pawn)
{
	FOnReadyData* FoundHandle = OnReadyHandles.FindByPredicate([&Pawn](const FOnReadyData& It)
	{
		return It.Pawn == &Pawn;
	});

	if (FoundHandle)
	{
		return *FoundHandle;
	}

	FOnReadyData NewHandle;
	NewHandle.Pawn = &Pawn;
	return OnReadyHandles.Emplace_GetRef(MoveTemp(NewHandle));
}