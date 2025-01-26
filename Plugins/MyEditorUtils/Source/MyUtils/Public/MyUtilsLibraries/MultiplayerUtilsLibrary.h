// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
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

	/** Returns amount of players (host + clients) playing this game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static int32 GetPlayersInMultiplayerNum();

	/** Returns the ping in milliseconds to the server in milliseconds.
	 * Is only valid on the local client, is 0 on the server or in standalone mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static float GetPingMs();
};
