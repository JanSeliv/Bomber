// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "MyBlueprintFunctionLibrary.generated.h"

enum class ELevelType : uint8;
enum class EActorType : uint8;

/**
 * 	The static functions library
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UMyBlueprintFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Static library functions
	 * --------------------------------------------------- */

	/** Returns number of alive players. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetAlivePlayersNum();

	/** Returns the type of the current level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static ELevelType GetLevelType();

	/* ---------------------------------------------------
	 *		Framework pointer getters
	 * --------------------------------------------------- */

	/** Returns the Bomber Game Mode, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class AMyGameModeBase* GetMyGameMode(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Bomber Game state, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class AMyGameStateBase* GetMyGameState(const UObject* OptionalWorldContext = nullptr);

	/** Returns the specified Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class AMyPlayerController* GetMyPlayerController(int32 PlayerIndex, const UObject* OptionalWorldContext = nullptr);

	/** Returns the local Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class AMyPlayerController* GetLocalPlayerController(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Bomber Player State for specified player, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetMyPlayerState(int32 CharacterID);

	/** Returns the player state of current controller. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class AMyPlayerState* GetLocalPlayerState(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Bomber settings. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class UMyGameUserSettings* GetMyGameUserSettings(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Settings widget. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class USettingsWidget* GetSettingsWidget(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Camera Component used on level. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class UMyCameraComponent* GetLevelCamera(const UObject* OptionalWorldContext = nullptr);

	/** Returns the HUD actor. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class AMyHUD* GetMyHUD(const UObject* OptionalWorldContext = nullptr);

	/** Returns the In-Game widget. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class UUserWidget* GetInGameWidget(const UObject* OptionalWorldContext = nullptr);

	/** Returns specified player character.
	 * @param CharacterID - Global ID of a character in session to find:
	 * INDEX_NONE - Local player, 0 - Host, 1 - client/AI1, 2 - client/AI2, 3 - client/AI3.
	 * @param OptionalWorldContext - the world context object.
	 * @warning CharacterID != PlayerIndex, so 0 will not return local player, but Host! */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class APlayerCharacter* GetPlayerCharacter(int32 CharacterID, const UObject* OptionalWorldContext = nullptr);

	/** Returns controlled player character. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class APlayerCharacter* GetLocalPlayerCharacter(const UObject* OptionalWorldContext = nullptr);

	/** Returns implemented Game Viewport Client on the project side. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UMyGameViewportClient* GetGameViewportClient();

	/** Returns the component that responsible for mouse-related logic like showing and hiding itself. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static class UMouseActivityComponent* GetMouseActivityComponent(const UObject* OptionalWorldContext = nullptr);

	/* ---------------------------------------------------
	*		EActorType functions
	* --------------------------------------------------- */

	/** Bitwise and(&) operation with bitmasks of actors types.
	 * Checks the actors types among each other between themselves */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "&"))
	static FORCEINLINE bool BitwiseActorTypes(
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 LBitmask,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 RBitmask)
	{
		return (LBitmask & RBitmask) != 0;
	}

	/** Returns Actor Type of specified actor, None is not level actor. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static EActorType GetActorType(const AActor* Actor);

	/** Returns true if specified actor is the Bomber Level Actor (player, box, wall or item). */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsLevelActor(const AActor* Actor);

	/** Returns true if specified level actor has at least one specified type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsActorHasAnyMatchingType(
		const AActor* Actor,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);
};