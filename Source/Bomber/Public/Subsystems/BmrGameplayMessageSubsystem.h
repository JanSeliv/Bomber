// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// Bomber
#include "Structures/BmrReadyHandler.h"

#include "BmrGameplayMessageSubsystem.generated.h"

/**
 * Global event system that lets any part of the game send and receive messages using gameplay tags.
 * Uses engine's Async Message System (aka Lyra's Gameplay Message Router) for tag-based message routing.
 */
UCLASS(BlueprintType, Blueprintable)
class BOMBER_API UBmrGameplayMessageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained.*/
	static UBmrGameplayMessageSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the pointer to this Subsystem. */
	static UBmrGameplayMessageSubsystem* GetGameplayMessageRouter(const UObject* OptionalWorldContext = nullptr);

	/** Broadcasts Gameplay Event Data via Async Message System (aka Lyra's Gameplay Message Router) and additionally forwards event to ASC if found from Target or Instigator.
	 * Is optional, UAsyncMessageWorldSubsystem::QueueMessageForBroadcast can be used directly.
	 * @param Payload Must contain valid EventTag.
	 * @param OptionalWorldContext If provided, uses this world to get the Message System; otherwise it automatically obtains the world (might be invalid during async actions). */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext, DisplayName = "BMR Broadcast Message"))
	static void BroadcastMessage(const struct FGameplayEventData& Payload, const UObject* OptionalWorldContext = nullptr);

	/** Registers a listener for the given event tag via Async Message System (aka Lyra's Gameplay Message Router).
	 * Is optional, prefer to use BIND_ON_ macros.
	 * Alternatively, UAsyncMessageWorldSubsystem::BindListener can be used directly.
	 * In blueprints, use `Start Listening for Async Message` node with the same Event Tag.
	 * @param WorldContext Used to obtain the world and its message system.
	 * @param EventTag The gameplay tag channel to listen on.
	 * @param Callback Invoked with extracted FGameplayEventData payload when a matching message is broadcast. */
	static void RegisterListener(const UObject* WorldContext, struct FGameplayTag EventTag, TFunction<void(const FGameplayEventData&)>&& Callback);

	/** Encapsulates the managements of 'On Player Ready' events.
	 * Contains various Broadcast_ methods. */
	FBmrReadyHandler ReadyHandler;

	/** Is called when this Subsystem is removed. */
	virtual void Deinitialize() override;
};

/*********************************************************************************************
 * Macro Helpers
 * All macros are optional, Async Message System (aka Lyra's Gameplay Message Router) can be used directly instead.
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
 * Obtain current state inside callback via Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu). */
#define BIND_ON_GAME_STATE_CHANGED(Obj, Function) \
	INTERNAL_BIND_GAME_STATE_CHANGED(BmrGameplayTags::Event::GameState_Changed, Obj, Function)

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

/** Internal macro for binding to game state changes via Async Message System (aka Lyra's Gameplay Message Router). */
#define INTERNAL_BIND_GAME_STATE_CHANGED(InEventTag, Obj, Function)                                             \
	{                                                                                                           \
		TWeakObjectPtr WeakObj(Obj);                                                                            \
		UBmrGameplayMessageSubsystem::RegisterListener(Obj, InEventTag,                                         \
		    [WeakObj](const FGameplayEventData& Payload)                                                        \
		{                                                                                                       \
			if (WeakObj.IsValid())                                                                              \
			{                                                                                                   \
				(WeakObj.Get()->*(&Function))(Payload);                                                         \
			}                                                                                                   \
		});                                                                                                     \
		const ABmrGameState* GameState = UBmrBlueprintFunctionLibrary::GetGameState();                          \
		if (GameState && GameState->HasMatchingGameplayTag(FBmrGameStateTag::ParentTag))                        \
		{                                                                                                       \
			FGameplayTagContainer OwnedTags;                                                                    \
			GameState->GetOwnedGameplayTags(OwnedTags);                                                         \
			FGameplayEventData AutoPayload;                                                                     \
			AutoPayload.EventTag = InEventTag;                                                                  \
			AutoPayload.InstigatorTags = OwnedTags.Filter(FBmrGameStateTag::ParentTag.GetSingleTagContainer()); \
			(Obj->*(&Function))(AutoPayload);                                                                   \
		}                                                                                                       \
	}

/** Internal macro for binding with ID filtering via Async Message System (aka Lyra's Gameplay Message Router). */
#define INTERNAL_BIND_READY_ID(InEventTag, Obj, Function, Arg, ID)                            \
	{                                                                                         \
		const int32 TargetPlayerId = ID;                                                      \
		TWeakObjectPtr WeakObj(Obj);                                                          \
		UBmrGameplayMessageSubsystem::RegisterListener(Obj, InEventTag,                       \
		    [WeakObj, TargetPlayerId](const FGameplayEventData& Payload)                      \
		{                                                                                     \
			if (!WeakObj.IsValid())                                                           \
			{                                                                                 \
				return;                                                                       \
			}                                                                                 \
			const APawn* CallbackPawn = Cast<APawn>(Payload.Instigator);                      \
			const APlayerState* PS = CallbackPawn ? CallbackPawn->GetPlayerState() : nullptr; \
			if (PS && PS->GetPlayerId() == TargetPlayerId)                                    \
			{                                                                                 \
				(WeakObj.Get()->*(&Function))(Payload);                                       \
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

/** Internal macro for binding with pointer filtering via Async Message System (aka Lyra's Gameplay Message Router). */
#define INTERNAL_BIND_READY_PTR(InEventTag, Obj, Function, TargetPtr)                \
	{                                                                                \
		if (TargetPtr)                                                               \
		{                                                                            \
			const ABmrPawn* Target = TargetPtr;                                      \
			TWeakObjectPtr WeakObj(Obj);                                             \
			UBmrGameplayMessageSubsystem::RegisterListener(Obj, InEventTag,          \
			    [WeakObj, Target](const FGameplayEventData& Payload)                 \
			{                                                                        \
				if (WeakObj.IsValid() && Payload.Instigator == Target)               \
				{                                                                    \
					(WeakObj.Get()->*(&Function))(Payload);                          \
				}                                                                    \
			});                                                                      \
			if (UBmrGameplayMessageSubsystem::Get(Obj).ReadyHandler.IsReady(Target)) \
			{                                                                        \
				FGameplayEventData AutoPayload;                                      \
				AutoPayload.EventTag = InEventTag;                                   \
				AutoPayload.Instigator = Target;                                     \
				(Obj->*(&Function))(AutoPayload);                                    \
			}                                                                        \
		}                                                                            \
	}

/** Internal macro for binding to local player events via Async Message System (aka Lyra's Gameplay Message Router). */
#define INTERNAL_BIND_READY_LOCAL(InEventTag, Obj, Function, Arg)                 \
	{                                                                             \
		TWeakObjectPtr WeakObj(Obj);                                              \
		UBmrGameplayMessageSubsystem::RegisterListener(Obj, InEventTag,           \
		    [WeakObj](const FGameplayEventData& Payload)                          \
		{                                                                         \
			if (!WeakObj.IsValid())                                               \
			{                                                                     \
				return;                                                           \
			}                                                                     \
			const APawn* CallbackPawn = Cast<APawn>(Payload.Instigator);          \
			if (CallbackPawn                                                      \
			    && CallbackPawn->IsLocallyControlled()                            \
			    && CallbackPawn->IsPlayerControlled())                            \
			{                                                                     \
				(WeakObj.Get()->*(&Function))(Payload);                           \
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