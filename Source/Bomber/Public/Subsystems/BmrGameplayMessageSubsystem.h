// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// Bomber
#include "Structures/BmrReadyHandler.h"

#include "BmrGlobalEventsSubsystem.generated.h"

/**
 * Contains gameplay delegates accessible from any place in the game.
 * Is much useful to keep delegates there instead of actors since it guarantees that they will be always available.
 */
UCLASS(BlueprintType, Blueprintable)
class BOMBER_API UBmrGlobalEventsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained.*/
	static UBmrGlobalEventsSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the pointer to this Subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UBmrGlobalEventsSubsystem* GetGlobalEventsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Game States
	 * - BIND_ON_GAME_STATE_CHANGED - called when the current game state was changed.
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EBmrCurrentGameState, CurrentGameState);

	/** Called when the current game state was changed.
	 * @warning in code, use BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]", DisplayName = "On Game State Changed")
	FOnGameStateChanged BP_OnGameStateChanged;

	/*********************************************************************************************
	 * On Player Ready
	 * Thsese delegates are managed by 'ReadyHandler'.
	 * @warning in code:
	 * - Instead of .Broadcast(), call BmrReadyHandler.Broadcast_ methods.
	 * - Instead of .AddDynamic(), use next macros:
	 * BIND_ON_PAWN_READY_ID(this, ThisClass::OnPawnReady, PlayerId);
	 * BIND_ON_PAWN_READY_PTR(this, ThisClass::OnPawnReady, Pawn);
	 * BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
	 * BIND_ON_PLAYER_STATE_READY_ID(this, ThisClass::OnPlayerStateReady, PlayerId);
	 * BIND_ON_PLAYER_STATE_READY_PTR(this, ThisClass::OnPlayerStateReady, PlayerState);
	 * BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);
	 ********************************************************************************************* */
public:
	/** Encapsulates the managements of 'On Player Ready' delegates.
	 * Contains various Broadcast_ methods. */
	FBmrReadyHandler ReadyHandler;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPawnReady, class ABmrPawn*, pawn, int32, PlayerId);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPawnReadyNative, ABmrPawn*, int32);

	/** Called when any pawn is spawned, possessed, and replicated.
	 * @warning in code, use BIND_ON_PAWN_READY_ID(this, &ThisClass::OnPawnReady, PlayerId) or BIND_ON_PAWN_READY_PTR(this, &ThisClass::OnPawnReady, pawnPtr); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "[Bomber]", DisplayName = "On Pawn Ready")
	FOnPawnReady BP_OnPawnReady;
	FOnPawnReadyNative OnPawnReadyNative;

	/** Called when the local player pawn is spawned, possessed, and replicated.
	 * Local means player with controller, so is broadcasted only once for each game instance (once on server and once on client).
	 * @warning in code, use BIND_ON_LOCAL_PAWN_READY(this, &ThisClass::OnLocalPawnReady); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "[Bomber]", DisplayName = "On Local Pawn Ready")
	FOnPawnReady BP_OnLocalPawnReady;
	FOnPawnReadyNative OnLocalPawnReadyNative;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateReady, class ABmrPlayerState*, PlayerState, int32, PlayerId);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateReadyNative, ABmrPlayerState*, int32);

	/** Called when any player state is initialized and its assigned pawn is ready.
	 * @warning in code, use BIND_ON_PLAYER_STATE_READY_ID(this, &ThisClass::OnPlayerStateReady, PlayerId) or BIND_ON_PLAYER_STATE_READY_PTR(this, &ThisClass::OnPlayerStateReady, PlayerStatePtr); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "[Bomber]", DisplayName = "On Player State Ready")
	FOnPlayerStateReady BP_OnPlayerStateReady;
	FOnPlayerStateReadyNative OnPlayerStateReadyNative;

	/** Called when the local player state is initialized and its assigned pawn is ready.
	 * Local means player with controller, so is broadcasted only once for each game instance (once on server and once on client).
	 * @warning in code, use BIND_ON_LOCAL_PLAYER_STATE_READY(this, &ThisClass::OnLocalPlayerStateReady); */
	UPROPERTY(BlueprintAssignable, Transient, Category = "[Bomber]", DisplayName = "On Local Player State Ready")
	FOnPlayerStateReady BP_OnLocalPlayerStateReady;
	FOnPlayerStateReadyNative OnLocalPlayerStateReadyNative;

	/*********************************************************************************************
	 * Other
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameProgressionCompleted);

	/** Called when a player completes the progression of the game */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]", DisplayName = "On Progression Completion")
	FOnGameProgressionCompleted OnGameProgressionCompleted;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called when this Subsystem is removed. */
	virtual void Deinitialize() override;
};

/** Helper macro to bind and call the function when the game state was changed. */
#define BIND_ON_GAME_STATE_CHANGED(Obj, Function)                                                \
	{                                                                                            \
		UBmrGlobalEventsSubsystem::Get().BP_OnGameStateChanged.AddUniqueDynamic(Obj, &Function); \
		if (ABmrGameState::GetCurrentGameState() == EBmrCurrentGameState::Menu)                  \
		{                                                                                        \
			Obj->Function(EBmrCurrentGameState::Menu);                                           \
		}                                                                                        \
	}

/*********************************************************************************************
 * Macro Helpers for Ready Events
 * - Auto-call if target already ready (vs regular binding that waits for future events)
 * - Filter to specific pawn/state (vs regular binding that broadcasts for ANY pawn)
 ********************************************************************************************* */

/** Helper macro for binding to player pawn ready events using PlayerId
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(ABmrPawn*, int32))
 * @param PlayerId pawn ID to wait for */
#define BIND_ON_PAWN_READY_ID(Obj, Function, PlayerId) \
	INTERNAL_BIND_LOCAL_READY_ID(OnPawnReadyNative, Obj, Function, Pawn, ABmrPawn, PlayerId)

/** Helper macro for binding to player pawn ready events using pointer
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(ABmrPawn*, int32))
 * @param Pawn Pointer to player pawn to wait for */
#define BIND_ON_PAWN_READY_PTR(Obj, Function, pawnPtr) \
	INTERNAL_BIND_LOCAL_READY_PTR(OnPawnReadyNative, Obj, Function, ABmrPawn, pawnPtr)

/** Helper macro for binding to local player pawn ready events
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(ABmrPawn*, int32)) */
#define BIND_ON_LOCAL_PAWN_READY(Obj, Function) \
	INTERNAL_BIND_LOCAL_READY(OnLocalPawnReadyNative, Obj, Function, Pawn, ABmrPawn)

/** Helper macro for binding to player state ready events using PlayerId
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(ABmrPlayerState*, int32))
 * @param PlayerId pawn ID to wait for */
#define BIND_ON_PLAYER_STATE_READY_ID(Obj, Function, PlayerId) \
	INTERNAL_BIND_LOCAL_READY_ID(OnPlayerStateReadyNative, Obj, Function, PlayerState, ABmrPlayerState, PlayerId)

/** Helper macro for binding to player state ready events using pointer
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(ABmrPlayerState*, int32))
 * @param PlayerState Pointer to player state to wait for */
#define BIND_ON_PLAYER_STATE_READY_PTR(Obj, Function, PlayerStatePtr) \
	INTERNAL_BIND_LOCAL_READY_PTR(OnPlayerStateReadyNative, Obj, Function, ABmrPlayerState, PlayerStatePtr)

/** Helper macro for binding to local player state ready events
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(ABmrPlayerState*, int32)) */
#define BIND_ON_LOCAL_PLAYER_STATE_READY(Obj, Function) \
	INTERNAL_BIND_LOCAL_READY(OnLocalPlayerStateReadyNative, Obj, Function, PlayerState, ABmrPlayerState)

/*********************************************************************************************
 * Internal
 ********************************************************************************************* */

/** Internal macro for binding and calling delegate methods with ID filtering. */
#define INTERNAL_BIND_LOCAL_READY_ID(NativeDelegate, Obj, Function, Arg, CallbackParamType, ID)                                     \
	{                                                                                                                               \
		const int32 TargetPlayerId = ID;                                                                                            \
		UBmrGlobalEventsSubsystem& EventsSubsystem = UBmrGlobalEventsSubsystem::Get(Obj);                                           \
		EventsSubsystem.NativeDelegate.AddWeakLambda(Obj, [Obj, TargetPlayerId](CallbackParamType* CallbackParam, int32 InPlayerId) \
		{                                                                                                                           \
			if (InPlayerId == TargetPlayerId)                                                                                       \
			{                                                                                                                       \
				(Obj->*(&Function))(CallbackParam, InPlayerId);                                                                     \
			}                                                                                                                       \
		});                                                                                                                         \
		auto* In##Arg = UBmrBlueprintFunctionLibrary::Get##Arg(TargetPlayerId);                                                     \
		if (EventsSubsystem.ReadyHandler.IsReady(In##Arg))                                                                          \
		{                                                                                                                           \
			(Obj->*(&Function))(In##Arg, TargetPlayerId);                                                                           \
		}                                                                                                                           \
	}

/** Internal macro for binding and calling delegate methods with pointer filtering. */
#define INTERNAL_BIND_LOCAL_READY_PTR(NativeDelegate, Obj, Function, CallbackParamType, TargetPtr)                              \
	{                                                                                                                           \
		if (TargetPtr)                                                                                                          \
		{                                                                                                                       \
			CallbackParamType* const Target = TargetPtr;                                                                        \
			const int32 PlayerId = Target->GetPlayerId();                                                                       \
			UBmrGlobalEventsSubsystem& EventsSubsystem = UBmrGlobalEventsSubsystem::Get(Obj);                                   \
			EventsSubsystem.NativeDelegate.AddWeakLambda(Obj, [Obj, Target](CallbackParamType* CallbackParam, int32 InPlayerId) \
			{                                                                                                                   \
				if (CallbackParam == Target)                                                                                    \
				{                                                                                                               \
					(Obj->*(&Function))(CallbackParam, InPlayerId);                                                             \
				}                                                                                                               \
			});                                                                                                                 \
			if (EventsSubsystem.ReadyHandler.IsReady(Target))                                                                   \
			{                                                                                                                   \
				(Obj->*(&Function))(Target, PlayerId);                                                                          \
			}                                                                                                                   \
		}                                                                                                                       \
	}

/** Internal macro for binding and calling local delegate methods. */
#define INTERNAL_BIND_LOCAL_READY(NativeDelegate, Obj, Function, Arg, CallbackParamType)                            \
	{                                                                                                               \
		UBmrGlobalEventsSubsystem& EventsSubsystem = UBmrGlobalEventsSubsystem::Get(Obj);                           \
		EventsSubsystem.NativeDelegate.AddWeakLambda(Obj, [Obj](CallbackParamType* CallbackParam, int32 InPlayerId) \
		{                                                                                                           \
			(Obj->*(&Function))(CallbackParam, InPlayerId);                                                         \
		});                                                                                                         \
		auto* In##Arg = UBmrBlueprintFunctionLibrary::Get##Arg(INDEX_NONE);                                         \
		if (EventsSubsystem.ReadyHandler.IsReady(In##Arg))                                                          \
		{                                                                                                           \
			const int32 PlayerId = In##Arg->GetPlayerId();                                                          \
			(Obj->*(&Function))(In##Arg, PlayerId);                                                                 \
		}                                                                                                           \
	}
