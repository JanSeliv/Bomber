// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "MultiplayerUtilsLibrary.generated.h"

/**
 * Function library with multiplayer-related helpers.
 *
 * Other useful multiplayer functions:
 * - UKismetSystemLibrary::IsServer
 * - UKismetSystemLibrary::HasMultipleLocalPlayers
 */
UCLASS()
class MYUTILS_API UMultiplayerUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if any client is connected to the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE bool IsMultiplayerGame() { return GetPlayersInMultiplayerNum() > 1; }

	/** Returns true if running from the server or party-leader client
	 * If false, then caller is joined client. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsPartyLeader();

	/** Returns true in editor game if running from client-only mode, so caller is single-player client or party-leader client. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsAuthoritativeClient();

	/** Returns amount of players (host + clients) playing this game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static int32 GetPlayersInMultiplayerNum();

	/** Returns the index of the current player during multiplayer.
	 * 0 is the server or party-leader client.
	 * 1 (or higher) is joined client. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static int32 GetPlayerId();

	/** Returns the ping in milliseconds to the server in milliseconds.
	 * Is only valid on the local client, is 0 on the server or in standalone mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static float GetPingMs();

	/** Returns player's ping in seconds.
	 * @param InPawn Pawn to retrieve player state and ping from. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static float GetPlayerPingSeconds(const APawn* InPawn);
};

/**
 * Overrides the replication parameters of an inherited property, including private ones.
 *
 * Useful for actors where certain engine properties have specific replication
 * parameters that need to be changed. This macro allows overriding their FDoRepLifetimeParams
 * (e.g., Condition, RepNotifyCondition, bIsPushBased, etc.) to customize replication behavior.
 *
 * Example 1: Enable APlayerState::bIsABot replication, which is originally COND_InitialOnly:
 *     FDoRepLifetimeParams Params;
 *     Params.Condition = COND_None;
 *     DOREPLIFETIME_OVERRIDE(Super, bIsABot, Params);
 *
 * Example 2: Override AActor::Instigator to always trigger RepNotify:
 *     FDoRepLifetimeParams Params;
 *     Params.RepNotifyCondition = REPNOTIFY_Always;
 *     DOREPLIFETIME_OVERRIDE(AActor, Instigator, Params);
 *
 * @param SuperClass     The class where the property is originally declared (e.g., AActor).
 * @param PropertyName   The name of the property to override.
 * @param Params         The FDoRepLifetimeParams to apply.
 */
#define DOREPLIFETIME_OVERRIDE(SuperClass, PropertyName, Params)                                                                         \
	{                                                                                                                                    \
		static const FName PropertyName##_PrivateName = TEXT(#PropertyName);                                                             \
		ResetReplicatedLifetimeProperty(StaticClass(), SuperClass::StaticClass(), PropertyName##_PrivateName, Params, OutLifetimeProps); \
	}