// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// Bomber
#include "Structures/OnCharactersReadyHandler.h"

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
	static UGlobalEventsSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

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
	 * BIND_ON_CHARACTER_READY_ID(this, ThisClass::OnCharacterReady, CharacterID);
	 * BIND_ON_CHARACTER_READY_PTR(this, ThisClass::OnCharacterReady, PlayerCharacter);
	 * BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnLocalCharacterReady);
	 * BIND_ON_PLAYER_STATE_READY_ID(this, ThisClass::OnPlayerStateReady, CharacterID);
	 * BIND_ON_PLAYER_STATE_READY_PTR(this, ThisClass::OnPlayerStateReady, PlayerState);
	 * BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);
	 ********************************************************************************************* */
public:
	/** Encapsulates the managements of 'On Player Ready' delegates.
	 * Contains various Broadcast_ methods. */
	FOnCharactersReadyHandler OnCharactersReadyHandler;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterReady, class APlayerCharacter*, Character, int32, CharacterID);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharacterReadyNative, APlayerCharacter*, int32);

	/** Called when any character is spawned, possessed, and replicated.
	 * @warning in code, use BIND_ON_CHARACTER_READY_ID(this, &ThisClass::OnCharacterReady, CharacterID) or BIND_ON_CHARACTER_READY_PTR(this, &ThisClass::OnCharacterReady, CharacterPtr); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Character Ready")
	FOnCharacterReady BP_OnCharacterReady;
	FOnCharacterReadyNative OnCharacterReadyNative;

	/** Called when the local player character is spawned, possessed, and replicated.
	 * Local means player with controller, so is broadcasted only once for each game instance (once on server and once on client).
	 * @warning in code, use BIND_ON_LOCAL_CHARACTER_READY(this, &ThisClass::OnLocalCharacterReady); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Local Character Ready")
	FOnCharacterReady BP_OnLocalCharacterReady;
	FOnCharacterReadyNative OnLocalCharacterReadyNative;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateReady, class AMyPlayerState*, PlayerState, int32, CharacterID);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateReadyNative, AMyPlayerState*, int32);

	/** Called when any player state is initialized and its assigned character is ready.
	 * @warning in code, use BIND_ON_PLAYER_STATE_READY_ID(this, &ThisClass::OnPlayerStateReady, CharacterID) or BIND_ON_PLAYER_STATE_READY_PTR(this, &ThisClass::OnPlayerStateReady, PlayerStatePtr); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Player State Ready")
	FOnPlayerStateReady BP_OnPlayerStateReady;
	FOnPlayerStateReadyNative OnPlayerStateReadyNative;

	/** Called when the local player state is initialized and its assigned character is ready.
	 * Local means player with controller, so is broadcasted only once for each game instance (once on server and once on client).
	 * @warning in code, use BIND_ON_LOCAL_PLAYER_STATE_READY(this, &ThisClass::OnLocalPlayerStateReady); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++", DisplayName = "On Local Player State Ready")
	FOnPlayerStateReady BP_OnLocalPlayerStateReady;
	FOnPlayerStateReadyNative OnLocalPlayerStateReadyNative;

	/*********************************************************************************************
	 * Other
	 ********************************************************************************************* */
public:
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

/** Helper macro to bind and call the function when the game state was changed. */
#define BIND_ON_GAME_STATE_CHANGED(Obj, Function)                                             \
	{                                                                                         \
		UGlobalEventsSubsystem::Get().BP_OnGameStateChanged.AddUniqueDynamic(Obj, &Function); \
		if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::Menu)               \
		{                                                                                     \
			Obj->Function(ECurrentGameState::Menu);                                           \
		}                                                                                     \
	}

/** Helper macro to bind and call the function when the game state actor was created. */
#define BIND_ON_GAME_STATE_CREATED(Obj, Function)                                        \
	{                                                                                    \
		if (AMyGameStateBase* GameState = UMyBlueprintFunctionLibrary::GetMyGameState()) \
		{                                                                                \
			Obj->Function(GameState);                                                    \
		}                                                                                \
		else if (UWorld* World = GetWorld())                                             \
		{                                                                                \
			World->GameStateSetEvent.AddUObject(Obj, &Function);                         \
		}                                                                                \
	}

/*********************************************************************************************
 * Macro Helpers for Ready Events
 * - Auto-call if target already ready (vs regular binding that waits for future events)
 * - Filter to specific character/state (vs regular binding that broadcasts for ANY character)
 ********************************************************************************************* */

/** Helper macro for binding to player character ready events using CharacterID
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(APlayerCharacter*, int32))
 * @param CharacterID Character ID to wait for */
#define BIND_ON_CHARACTER_READY_ID(Obj, Function, CharacterID) \
	INTERNAL_BIND_CHARACTER_READY_ID(OnCharacterReadyNative, Obj, Function, PlayerCharacter, APlayerCharacter, CharacterID)

/** Helper macro for binding to player character ready events using pointer
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(APlayerCharacter*, int32))
 * @param PlayerCharacter Pointer to player character to wait for */
#define BIND_ON_CHARACTER_READY_PTR(Obj, Function, CharacterPtr) \
	INTERNAL_BIND_CHARACTER_READY_PTR(OnCharacterReadyNative, Obj, Function, APlayerCharacter, CharacterPtr)

/** Helper macro for binding to local player character ready events
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(APlayerCharacter*, int32)) */
#define BIND_ON_LOCAL_CHARACTER_READY(Obj, Function) \
	INTERNAL_BIND_LOCAL_READY(OnLocalCharacterReadyNative, Obj, Function, PlayerCharacter, APlayerCharacter)

/** Helper macro for binding to player state ready events using CharacterID
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(AMyPlayerState*, int32))
 * @param CharacterID Character ID to wait for */
#define BIND_ON_PLAYER_STATE_READY_ID(Obj, Function, CharacterID) \
	INTERNAL_BIND_CHARACTER_READY_ID(OnPlayerStateReadyNative, Obj, Function, MyPlayerState, AMyPlayerState, CharacterID)

/** Helper macro for binding to player state ready events using pointer
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(AMyPlayerState*, int32))
 * @param PlayerState Pointer to player state to wait for */
#define BIND_ON_PLAYER_STATE_READY_PTR(Obj, Function, PlayerStatePtr) \
	INTERNAL_BIND_CHARACTER_READY_PTR(OnPlayerStateReadyNative, Obj, Function, AMyPlayerState, PlayerStatePtr)

/** Helper macro for binding to local player state ready events
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(AMyPlayerState*, int32)) */
#define BIND_ON_LOCAL_PLAYER_STATE_READY(Obj, Function) \
	INTERNAL_BIND_LOCAL_READY(OnLocalPlayerStateReadyNative, Obj, Function, MyPlayerState, AMyPlayerState)

/*********************************************************************************************
 * Internal
 ********************************************************************************************* */

/** Internal macro for binding and calling delegate methods with ID filtering. */
#define INTERNAL_BIND_CHARACTER_READY_ID(NativeDelegate, Obj, Function, Arg, CallbackParamType, ID)                                       \
	{                                                                                                                                     \
		const int32 TargetCharacterID = ID;                                                                                               \
		UGlobalEventsSubsystem& EventsSubsystem = UGlobalEventsSubsystem::Get(Obj);                                                       \
		EventsSubsystem.NativeDelegate.AddWeakLambda(Obj, [Obj, TargetCharacterID](CallbackParamType* CallbackParam, int32 InCharacterID) \
		{                                                                                                                                 \
			if (InCharacterID == TargetCharacterID)                                                                                       \
			{                                                                                                                             \
				(Obj->*(&Function))(CallbackParam, InCharacterID);                                                                        \
			}                                                                                                                             \
		});                                                                                                                               \
		auto* Arg = UMyBlueprintFunctionLibrary::Get##Arg(TargetCharacterID);                                                             \
		if (EventsSubsystem.OnCharactersReadyHandler.IsCharacterReady(Arg))                                                               \
		{                                                                                                                                 \
			(Obj->*(&Function))(Arg, TargetCharacterID);                                                                                  \
		}                                                                                                                                 \
	}

/** Internal macro for binding and calling delegate methods with pointer filtering. */
#define INTERNAL_BIND_CHARACTER_READY_PTR(NativeDelegate, Obj, Function, CallbackParamType, TargetPtr)                             \
	{                                                                                                                              \
		if (TargetPtr)                                                                                                             \
		{                                                                                                                          \
			CallbackParamType* const Target = TargetPtr;                                                                           \
			const int32 CharacterID = Target->GetPlayerId();                                                                       \
			UGlobalEventsSubsystem& EventsSubsystem = UGlobalEventsSubsystem::Get(Obj);                                            \
			EventsSubsystem.NativeDelegate.AddWeakLambda(Obj, [Obj, Target](CallbackParamType* CallbackParam, int32 InCharacterID) \
			{                                                                                                                      \
				if (CallbackParam == Target)                                                                                       \
				{                                                                                                                  \
					(Obj->*(&Function))(CallbackParam, InCharacterID);                                                             \
				}                                                                                                                  \
			});                                                                                                                    \
			if (EventsSubsystem.OnCharactersReadyHandler.IsCharacterReady(Target))                                                 \
			{                                                                                                                      \
				(Obj->*(&Function))(Target, CharacterID);                                                                          \
			}                                                                                                                      \
		}                                                                                                                          \
	}

/** Internal macro for binding and calling local delegate methods. */
#define INTERNAL_BIND_LOCAL_READY(NativeDelegate, Obj, Function, Arg, CallbackParamType)                               \
	{                                                                                                                  \
		UGlobalEventsSubsystem& EventsSubsystem = UGlobalEventsSubsystem::Get(Obj);                                    \
		EventsSubsystem.NativeDelegate.AddWeakLambda(Obj, [Obj](CallbackParamType* CallbackParam, int32 InCharacterID) \
		{                                                                                                              \
			(Obj->*(&Function))(CallbackParam, InCharacterID);                                                         \
		});                                                                                                            \
		auto* Arg = UMyBlueprintFunctionLibrary::Get##Arg(INDEX_NONE);                                                 \
		if (EventsSubsystem.OnCharactersReadyHandler.IsCharacterReady(Arg))                                            \
		{                                                                                                              \
			const int32 CharacterID = Arg->GetPlayerId();                                                              \
			(Obj->*(&Function))(Arg, CharacterID);                                                                     \
		}                                                                                                              \
	}
