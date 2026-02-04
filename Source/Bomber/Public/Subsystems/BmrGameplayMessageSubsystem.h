// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/GameplayMessageSubsystem.h"

// Bomber
#include "Structures/BmrReadyHandler.h"

#include "BmrGameplayMessageSubsystem.generated.h"

/**
 * Gameplay Message Router is a global event system that lets any part of the game send and receive messages using gameplay tags.
 * Inherits Gameplay Message Router for own helpers.
 * @see https://github.com/JanSeliv/GameplayMessageRouter for Getting Started and general usage.
 */
UCLASS(BlueprintType, Blueprintable)
class BOMBER_API UBmrGameplayMessageSubsystem : public UGameplayMessageSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained.*/
	static UBmrGameplayMessageSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the pointer to this Subsystem. */
	static UBmrGameplayMessageSubsystem* GetGameplayMessageRouter(const UObject* OptionalWorldContext = nullptr);

	/** Alternative version of templated UGameplayMessageSubsystem::BroadcastMessage, which always accepts Gameplay Event Data to additionally broadcast event to  ASC if found from Target or Instigator.
	 * If you want to broadcast other type of message, use UGameplayMessageSubsystem::Get(this).BroadcastMessage<YourType>(...) instead, but it won't be forwarded to ASC.
	 * @param Payload Must contain valid EventTag.
	 * @param OptionalWorldContext If provided, uses this world to get the Message Router; otherwise it automatically obtains the world (might be invalid during async actions). */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext, DisplayName = "BMR Broadcast Message"))
	static void BroadcastMessage(const struct FGameplayEventData& Payload, const UObject* OptionalWorldContext = nullptr);

	/** Encapsulates the managements of 'On Player Ready' events.
	 * Contains various Broadcast_ methods. */
	FBmrReadyHandler ReadyHandler;

	/** Is called when this Subsystem is removed. */
	virtual void Deinitialize() override;
};

/*********************************************************************************************
 * Macro Helpers
 * All macros are optional, Gameplay Message Router can be used directly instead.
 * They provide additional benefits in code, such as:
 * - Auto-call if target already ready (vs regular binding that waits for future events)
 * - Filter to specific pawn/state (vs regular binding that broadcasts for ANY pawn)
 * Callback signature: void Function(const FGameplayEventData& Payload)
 * Next macros are currently available:
 * BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
 * BIND_ON_PAWN_READY_ID(this, ThisClass::OnPawnReady, PlayerId);
 * BIND_ON_PAWN_READY_PTR(this, ThisClass::OnPawnReady, Pawn);
 * BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
 ********************************************************************************************* */

/** Helper macro to bind and call the function when the game state was changed.
 * Obtain current state inside callback via ABmrGameState::GetCurrentGameState(). */
#define BIND_ON_GAME_STATE_CHANGED(Obj, Function)                                    \
	{                                                                                \
		TWeakObjectPtr WeakObj(Obj);                                                 \
		UGameplayMessageSubsystem::Get(Obj).RegisterListener<FGameplayEventData>( \
		    BmrGameplayTags::Event::GameState_Changed,                               \
		    [WeakObj](FGameplayTag, const FGameplayEventData& Payload) {             \
			if (auto* StrongObj = WeakObj.Get())                                     \
			{                                                                        \
				(StrongObj->*(&Function))(Payload);                                  \
			}                                                                        \
		});                                                                          \
		if (ABmrGameState::GetCurrentGameState() == EBmrCurrentGameState::Menu)      \
		{                                                                            \
			FGameplayEventData AutoPayload;                                          \
			AutoPayload.EventTag = BmrGameplayTags::Event::GameState_Changed;        \
			(Obj->*(&Function))(AutoPayload);                                        \
		}                                                                            \
	}

/** Helper macro for binding to player pawn ready events using PlayerId.
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(const FGameplayEventData& Payload))
 * @param PlayerId pawn ID to wait for */
#define BIND_ON_PAWN_READY_ID(Obj, Function, PlayerId) \
	INTERNAL_BIND_READY_ID(BmrGameplayTags::Event::Player_PawnReady, Obj, Function, Pawn, PlayerId)

/** Helper macro for binding to player pawn ready events using pointer.
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(const FGameplayEventData& Payload))
 * @param PawnPtr Pointer to player pawn to wait for */
#define BIND_ON_PAWN_READY_PTR(Obj, Function, PawnPtr) \
	INTERNAL_BIND_READY_PTR(BmrGameplayTags::Event::Player_PawnReady, Obj, Function, PawnPtr)

/** Helper macro for binding to local player pawn ready events.
 * @param Obj Object that owns the callback function
 * @param Function Callback function to bind (signature: void Function(const FGameplayEventData& Payload)) */
#define BIND_ON_LOCAL_PAWN_READY(Obj, Function) \
	INTERNAL_BIND_READY_LOCAL(BmrGameplayTags::Event::Player_PawnReady, Obj, Function, Pawn)

/*********************************************************************************************
 * Internal
 ********************************************************************************************* */

/** Internal macro for binding with ID filtering via Gameplay Message Router. */
#define INTERNAL_BIND_READY_ID(InEventTag, Obj, Function, Arg, ID)                            \
	{                                                                                         \
		const int32 TargetPlayerId = ID;                                                      \
		TWeakObjectPtr WeakObj(Obj);                                                          \
		UGameplayMessageSubsystem::Get(Obj).RegisterListener<FGameplayEventData>(             \
		    InEventTag,                                                                       \
		    [WeakObj, TargetPlayerId](FGameplayTag, const FGameplayEventData& Payload)        \
		{                                                                                     \
			auto* StrongObj = WeakObj.Get();                                                  \
			const APawn* CallbackPawn = Cast<APawn>(Payload.Instigator);                      \
			const APlayerState* PS = CallbackPawn ? CallbackPawn->GetPlayerState() : nullptr; \
			if (StrongObj && PS && PS->GetPlayerId() == TargetPlayerId)                       \
			{                                                                                 \
				(StrongObj->*(&Function))(Payload);                                           \
			}                                                                                 \
		});                                                                                   \
		ABmrPawn* In##Arg = UBmrBlueprintFunctionLibrary::Get##Arg(TargetPlayerId);           \
		if (UBmrGameplayMessageSubsystem::Get(Obj).ReadyHandler.IsReady(In##Arg))             \
		{                                                                                     \
			FGameplayEventData AutoPayload;                                                   \
			AutoPayload.EventTag = InEventTag;                                                \
			AutoPayload.Instigator = In##Arg;                                                 \
			(Obj->*(&Function))(AutoPayload);                                                 \
		}                                                                                     \
	}

/** Internal macro for binding with pointer filtering via Gameplay Message Router. */
#define INTERNAL_BIND_READY_PTR(InEventTag, Obj, Function, TargetPtr)                 \
	{                                                                                 \
		if (TargetPtr)                                                                \
		{                                                                             \
			auto* const Target = TargetPtr;                                           \
			TWeakObjectPtr WeakObj(Obj);                                              \
			UGameplayMessageSubsystem::Get(Obj).RegisterListener<FGameplayEventData>( \
			    InEventTag,                                                           \
			    [WeakObj, Target](FGameplayTag, const FGameplayEventData& Payload)    \
			{                                                                         \
				auto* StrongObj = WeakObj.Get();                                      \
				if (StrongObj && Payload.Instigator == Target)                        \
				{                                                                     \
					(StrongObj->*(&Function))(Payload);                               \
				}                                                                     \
			});                                                                       \
			if (UBmrGameplayMessageSubsystem::Get(Obj).ReadyHandler.IsReady(Target))  \
			{                                                                         \
				FGameplayEventData AutoPayload;                                       \
				AutoPayload.EventTag = InEventTag;                                    \
				AutoPayload.Instigator = Target;                                      \
				(Obj->*(&Function))(AutoPayload);                                     \
			}                                                                         \
		}                                                                             \
	}

/** Internal macro for binding to local player events via Gameplay Message Router. */
#define INTERNAL_BIND_READY_LOCAL(InEventTag, Obj, Function, Arg)                 \
	{                                                                             \
		TWeakObjectPtr WeakObj(Obj);                                              \
		UGameplayMessageSubsystem::Get(Obj).RegisterListener<FGameplayEventData>( \
		    InEventTag,                                                           \
		    [WeakObj](FGameplayTag, const FGameplayEventData& Payload)            \
		{                                                                         \
			auto* StrongObj = WeakObj.Get();                                      \
			const APawn* CallbackPawn = Cast<APawn>(Payload.Instigator);          \
			if (StrongObj && CallbackPawn                                         \
			    && CallbackPawn->IsLocallyControlled()                            \
			    && CallbackPawn->IsPlayerControlled())                            \
			{                                                                     \
				(StrongObj->*(&Function))(Payload);                               \
			}                                                                     \
		});                                                                       \
		ABmrPawn* In##Arg = UBmrBlueprintFunctionLibrary::GetLocalPawn();         \
		if (UBmrGameplayMessageSubsystem::Get(Obj).ReadyHandler.IsReady(In##Arg)) \
		{                                                                         \
			FGameplayEventData AutoPayload;                                       \
			AutoPayload.EventTag = InEventTag;                                    \
			AutoPayload.Instigator = In##Arg;                                     \
			(Obj->*(&Function))(AutoPayload);                                     \
		}                                                                         \
	}
