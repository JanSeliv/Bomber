// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "EditorUtilsLibrary.generated.h"

/**
 * The editor functions library
 */
UCLASS()
class MYEDITORUTILS_API UEditorUtilsLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Checks, is the current world placed in the editor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditor();

	/** Checks is the current world placed in the editor and the game not started yet. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorNotPieWorld();

	/** Returns true if game is started in the Editor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsPIE();

	/** Returns true if is started multiplayer game (server + client(s)) right in the Editor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorMultiplayer();

	/** Returns the index of current player during editor multiplayer.
	 * 0 is server.
	 * 1 (or higher) is client.
	 * -1 in the standalone game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static int32 GetEditorPlayerIndex();
};
