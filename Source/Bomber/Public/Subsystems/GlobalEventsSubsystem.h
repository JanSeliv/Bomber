// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "Structures/OnCharactersReadyHandler.h"
//---
#include "GlobalEventsSubsystem.generated.h"

/**
 * Contains gameplay delegates accessible from any place in the game.
 * Is much useful to keep delegates there instead of actors since it guarantees that they will be always available.
 */
UCLASS(BlueprintType, Blueprintable)
class BOMBER_API UGlobalEventsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained.*/
	static UGlobalEventsSubsystem& Get();

	/** Returns the pointer to this Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UGlobalEventsSubsystem* GetGlobalEventsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Game States
	 * - BIND_ON_GAME_STATE_CHANGED - called when the current game state was changed.
	 * - BIND_ON_GAME_STATE_CREATED - called when the game state actor was created. 
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);

	/** Called when the current game state was changed.
	 * @warning in code, use BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Game State Changed")
	FOnGameStateChanged BP_OnGameStateChanged;

	/*********************************************************************************************
	 * On Player Ready
	 * Thsese delegates are managed by 'OnCharactersReadyHandler'.
     * @warning in code:
     * - Instead of .Broadcast(), call OnCharactersReadyHandler.Broadcast_ methods.
     * - Instead of .AddDynamic(), use next macros:
	 * BIND_ON_CHARACTER_READY(this, ThisClass::OnCharacterReady, CharacterID);
	 * BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnLocalCharacterReady);
	 * BIND_ON_PLAYER_STATE_READY(this, ThisClass::OnPlayerStateReady, CharacterID);
	 * BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);
	 ********************************************************************************************* */
public:
	/** Encapsulates the managements of 'On Player Ready' delegates.
	 * Contains various Broadcast_ methods. */
	FOnCharactersReadyHandler OnCharactersReadyHandler;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterReady, class APlayerCharacter*, Character, int32, CharacterID);

	/** Called when any character is spawned, possessed, and replicated.
	 * @warning in code, use BIND_ON_CHARACTER_READY(this, ThisClass::OnCharacterReady, CharacterID); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Character Ready")
	FOnCharacterReady BP_OnCharacterReady;

	/** Called when the local player character is spawned, possessed, and replicated.
	 * @warning in code, use BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnLocalCharacterReady); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Local Character Ready")
	FOnCharacterReady BP_OnLocalCharacterReady;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateReady, class AMyPlayerState*, PlayerState, int32, CharacterID);

	/** Called when any player state is initialized and its assigned character is ready.
	 * @warning in code, use BIND_ON_PLAYER_STATE_READY(this, ThisClass::OnPlayerStateReady, CharacterID); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Player State Ready")
	FOnPlayerStateReady BP_OnPlayerStateReady;

	/** Called when the local player state is initialized and its assigned character is ready.
	 * @warning in code, use BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Local Player State Ready")
	FOnPlayerStateReady BP_OnLocalPlayerStateReady;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameProgressionCompleted);

	/** Called when a player completes the progression of the game */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Progression Completion")
	FOnGameProgressionCompleted OnGameProgressionCompleted;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called when this Subsystem is removed. */
	virtual void Deinitialize() override;
};

/*********************************************************************************************
 * Macro Helpers
 * In general, used to additionally call given function if event was broadcasted before binding.
 ********************************************************************************************* */

/** Helper macro to bind and call the function when the game state was changed. */
#define BIND_ON_GAME_STATE_CHANGED(Obj, Function) \
{ \
	UGlobalEventsSubsystem::Get().BP_OnGameStateChanged.AddUniqueDynamic(Obj, &Function); \
	if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::Menu) \
	{ \
		Obj->Function(ECurrentGameState::Menu); \
	} \
}

/** Helper macro to bind and call the function when the game state actor was created. */
#define BIND_ON_GAME_STATE_CREATED(Obj, Function) \
{ \
	if (AMyGameStateBase* GameState = UMyBlueprintFunctionLibrary::GetMyGameState()) \
	{ \
		Obj->Function(GameState); \
	} \
	else if (UWorld* World = GetWorld()) \
	{ \
		World->GameStateSetEvent.AddUObject(Obj, &Function); \
	} \
}

/** Helper macro to bind (or call if possible) when any character is spawned, possessed, and replicated. */
#define BIND_ON_CHARACTER_READY(Obj, Function, CharacterID) \
    INTERNAL_BIND_CHARACTER_READY(BP_OnCharacterReady, Obj, Function, PlayerCharacter, CharacterID)

/** Helper macro to bind (or call if possible) when the local player character is spawned, possessed, and replicated. */
#define BIND_ON_LOCAL_CHARACTER_READY(Obj, Function) \
    INTERNAL_BIND_CHARACTER_READY(BP_OnLocalCharacterReady, Obj, Function, PlayerCharacter, INDEX_NONE)

/** Helper macro to bind (or call if possible) when any player state is initialized and its assigned character is ready. */
#define BIND_ON_PLAYER_STATE_READY(Obj, Function, CharacterID) \
    INTERNAL_BIND_CHARACTER_READY(BP_OnPlayerStateReady, Obj, Function, MyPlayerState, CharacterID)

/** Helper macro to bind (or call if possible) when the local player state is initialized and its assigned character is ready. */
#define BIND_ON_LOCAL_PLAYER_STATE_READY(Obj, Function) \
    INTERNAL_BIND_CHARACTER_READY(BP_OnLocalPlayerStateReady, Obj, Function, MyPlayerState, INDEX_NONE)