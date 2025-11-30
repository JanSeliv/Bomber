// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BmrBlueprintFunctionLibrary.generated.h"

enum class EBmrLevelType : uint8;
enum class EBmrActorType : uint8;
enum class EBmrPlayerType : uint8;

/**
 * 	The static functions library
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrBlueprintFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Static library functions
	 * --------------------------------------------------- */

	/** Returns number of alive players.
	 * @param InPlayerType - the type of the characters to count. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static int32 GetAlivePlayersNum(EBmrPlayerType InPlayerType);

	/** Returns the type of the current level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static EBmrLevelType GetLevelType();

	/* ---------------------------------------------------
	 *		Framework pointer getters
	 * --------------------------------------------------- */

	/** Returns the Bomber Game Mode, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Game Mode", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrGameMode* GetGameMode(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Bomber Game state, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Game State", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrGameState* GetGameState(const UObject* OptionalWorldContext = nullptr);

	/** Returns the specified Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Player Controller", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrPlayerController* GetPlayerController(int32 PlayerIndex, const UObject* OptionalWorldContext = nullptr);

	/** Returns the local Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Player Controller (Local)", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrPlayerController* GetLocalPlayerController(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Bomber Player State for specified player, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Player State", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrPlayerState* GetPlayerState(int32 PlayerId, const UObject* OptionalWorldContext = nullptr);

	/** Returns the player state of current controller. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Player State (Local)", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrPlayerState* GetLocalPlayerState(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Bomber settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Game User Settings", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UBmrGameUserSettings* GetGameUserSettings(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class USettingsWidget* GetSettingsWidget(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Camera Component used on level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UBmrCameraComponent* GetLevelCamera(const UObject* OptionalWorldContext = nullptr);

	/** Returns specified player character.
	 * @param PlayerId - Global ID of a character in session to find:
	 * INDEX_NONE - Local player, 0 - Host, 1 - client/AI1, 2 - client/AI2, 3 - client/AI3.
	 * @param OptionalWorldContext - the world context object.
	 * @warning PlayerId != PlayerIndex, so 0 will not return local player, but Host! */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Pawn", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrPawn* GetPawn(int32 PlayerId, const UObject* OptionalWorldContext = nullptr);

	/** Returns controlled player character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Pawn (Local)", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class ABmrPawn* GetLocalPawn(const UObject* OptionalWorldContext = nullptr);

	/** Returns specified Ability System Component.
	 * @param PlayerId - Global ID of a character in session to find.
	 * @param OptionalWorldContext - the world context object. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UAbilitySystemComponent* GetAbilitySystemComponent(int32 PlayerId, const UObject* OptionalWorldContext = nullptr);

	/** Returns the Ability System Component from the local Player State. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UAbilitySystemComponent* GetLocalAbilitySystemComponent(const UObject* OptionalWorldContext = nullptr);

	/** Returns specified Mover Component.
	 * @param PlayerId - Global ID of a character in session to find.
	 * @param OptionalWorldContext - the world context object. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Mover Component", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UBmrMoverComponent* GetMoverComponent(int32 PlayerId, const UObject* OptionalWorldContext = nullptr);

	/** Returns the Mover Component from the local Player Character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Mover Component (Local)", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UBmrMoverComponent* GetLocalMoverComponent(const UObject* OptionalWorldContext = nullptr);

	/** Returns implemented Game Viewport Client on the project side. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", DisplayName = "Get BMR Game Viewport Client")
	static class UBmrGameViewportClient* GetGameViewportClient();

	/** Returns the component that responsible for mouse-related logic like showing and hiding itself. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UBmrMouseActivityComponent* GetMouseActivityComponent(const UObject* OptionalWorldContext = nullptr);

	/* ---------------------------------------------------
	 *		EBmrActorType functions
	 * --------------------------------------------------- */

	/** Bitwise and(&) operation with bitmasks of actors types.
	 * Checks the actors types among each other between themselves */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (CompactNodeTitle = "&"))
	static FORCEINLINE bool BitwiseActorTypes(
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 LBitmask,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 RBitmask)
	{
		return (LBitmask & RBitmask) != 0;
	}

	/** Returns Actor Type of specified actor, None is not level actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static EBmrActorType GetActorType(const AActor* Actor);

	/** Returns true if specified actor is the Bomber Level Actor (player, box, wall or item). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static bool IsLevelActor(const AActor* Actor);

	/** Returns true if specified level actor has at least one specified type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static bool IsActorHasAnyMatchingType(
	    const AActor* Actor,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);
};