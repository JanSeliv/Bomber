// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// UE
#include "Abilities/GameplayAbilityTypes.h" // FGameplayEventData
#include "AsyncMessageHandle.h"

#include "GlobalMessageSubsystem.generated.h"

/** Called when a global message is received with the gameplay event payload */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGlobalMessageReceived, const FGameplayEventData&, Payload);

/**
 * World subsystem providing CallOr pattern on top of engine's Async Message System.
 * Caches broadcast events and replays them to late subscribers, eliminating the need for per-event readiness checks.
 * Uses internally engine's Async Message System (aka Lyra Gameplay Message Router)
 */
UCLASS(BlueprintType, Blueprintable)
class MYUTILS_API UGlobalMessageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and will crash if can't be obtained */
	static UGlobalMessageSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the pointer to this Subsystem, nullptr if world is not available */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Global Messages",
	    meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UGlobalMessageSubsystem* GetGlobalMessageSubsystem(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Listeners
	 ********************************************************************************************* */
public:
	/** Blueprint-only listener node, wraps CallOrStartListeningForGlobalMessage.
	 * In code use the templated CallOrStartListeningForGlobalMessage() instead.
	 * Returns handle that can be used to unbind the listener later */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (DisplayName = "Call Or Start Listening For Global Message", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static FAsyncMessageHandle BPCallOrStartListeningForGlobalMessage(UObject* WorldContextObject, FGameplayTag MessageTag, const FOnGlobalMessageReceived& Completed);

	/** Subscribes to an event by function-callback.
	 * If the event was already broadcast, fires the callback immediately with the cached data.
	 * Returns handle that can be used to unbind the listener later.
	 * Example: UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(Tag, this, &ThisClass::OnEvent); */
	template <typename TOwner>
	static FAsyncMessageHandle CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, TOwner* Object, void (TOwner::*Function)(const FGameplayEventData&));

	/** Subscribes to an event via lambda-callback with weak object safety.
	 * Returns handle that can be used to unbind the listener later.
	 * Example: UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(Tag, this, [this](const FGameplayEventData& Payload){ ... }); */
	static FAsyncMessageHandle CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, const UObject* ListenerOwner, TFunction<void(const FGameplayEventData&)>&& Callback);

	/** Unbinds a listener so it will no longer receive callbacks.
	 * Uses the handle returned by CallOrStartListeningForGlobalMessage */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static void StopListeningForGlobalMessage(const FAsyncMessageHandle& Handle, const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Broadcast
	 ********************************************************************************************* */
public:
	/** Broadcasts Gameplay Event Data via engine's Async Message System, caches the event for the CallOr pattern,
	 * and additionally forwards event to Ability System Component if found from Payload.Target or Payload.Instigator.
	 * Is optional wrapper, engine's QueueMessageForBroadcast can be used directly if CallOr caching and ASC forwarding are not needed */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static void BroadcastGlobalMessage(const FGameplayEventData& Payload, const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Clears cached broadcast data */
	virtual void Deinitialize() override;

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Cached broadcast data enabling the CallOr pattern for late subscribers.
	 * Outer map: gameplay tag identifying the event channel.
	 * Inner map: keyed by instigator actor so each unique broadcaster keeps its own cached payload.
	 * - Same instigator broadcasting again overwrites its previous entry (e.g., game state changes keeps only latest state).
	 * - Different instigators accumulate (e.g., 4 pawns broadcasting player state change each get their own cached entry).
	 * When a late subscriber binds, all cached entries for the tag are replayed, so listeners can filter the correct instigator */
	TMap<FGameplayTag, TMap<TWeakObjectPtr<const AActor> /*Instigator*/, FGameplayEventData>> BroadcastedMessagesMap;
};

// Subscribes to a gameplay event via member function with weak object safety
template <typename TOwner>
FAsyncMessageHandle UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, TOwner* Object, void (TOwner::*Function)(const FGameplayEventData&))
{
	TWeakObjectPtr<TOwner> WeakObject(Object);
	return CallOrStartListeningForGlobalMessage(MessageTag, Object, [WeakObject, Function](const FGameplayEventData& Payload)
	{
		if (TOwner* StrongObject = WeakObject.Get())
		{
			(StrongObject->*Function)(Payload);
		}
	});
}
