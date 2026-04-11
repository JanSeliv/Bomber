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
 * World subsystem which uses internally and extends Unreal's Async Message System (aka Lyra Gameplay Message Router).
 * Caches broadcast events and replays them to late subscribers, eliminating the need for per-event readiness checks.
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
	 * In code use the templated CallOrStartListeningForGlobalMessage() instead */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (DisplayName = "Call Or Start Listening For Global Message", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static void BPCallOrStartListeningForGlobalMessage(UObject* WorldContextObject, FGameplayTag MessageTag, const FOnGlobalMessageReceived& Completed);

	/** Subscribes to an event by function-callback.
	 * If the event was already broadcast, fires the callback immediately with the cached data.
	 * Example: UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(Tag, this, &ThisClass::OnEvent); */
	template <typename TOwner>
	static void CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, TOwner* Object, void (TOwner::*Function)(const FGameplayEventData&));

	/** Subscribes to an event via lambda-callback with weak object safety.
	 * Example: UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(Tag, this, [this](const FGameplayEventData& Payload){ ... }); */
	static void CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, const UObject* ListenerOwner, TFunction<void(const FGameplayEventData&)>&& Callback);

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
	 * Unbind
	 ********************************************************************************************* */
public:
	/** Unbinds a single event for the given listener while keeping all other events this listener is subscribed to.
	 * Use when the listener remains alive but no longer needs this specific event */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (WorldContext = "ListenerOwner", CallableWithoutWorldContext))
	static void StopListeningForGlobalMessage(FGameplayTag MessageTag, const UObject* ListenerOwner);

	/** Unbinds all events for the given listener at once.
	 * Use at the end of the listener's lifetime (EndPlay, OnUnregister, Deinitialize) when the object is about to be destroyed */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (WorldContext = "ListenerOwner", CallableWithoutWorldContext))
	static void StopListeningForAllGlobalMessages(const UObject* ListenerOwner);

	/** Clears all cached broadcast data for the given tag, restarting the logical session.
	 * Useful for events owned by modular game features that can unload and need to clear their cache,
	 * so when the feature loads again, early-binding listeners receive fresh data instead of stale payloads from the previous activation.
	 * Should be called when the broadcaster's lifecycle restarts within the same world session (e.g., in MGF OnDeactivating) */
	UFUNCTION(BlueprintCallable, Category = "Global Messages", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static void ClearCachedMessages(FGameplayTag MessageTag, const UObject* OptionalWorldContext = nullptr);

	/** Returns true if the given event has already been broadcast at least once this session.
	 * Is useful instead of adding a separate bool to track whether something already happened, since the event cache already holds that knowledge */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Global Messages", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static bool HasBroadcastedMessage(FGameplayTag MessageTag, const UObject* OptionalWorldContext = nullptr);

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

	/** Internal registry mapping listener owners to their engine handles per tag, enabling unbind-by-owner. */
	TMap<TWeakObjectPtr<const UObject> /*ListenerOwner*/, TMap<FGameplayTag, FAsyncMessageHandle>> ListenerHandlesMap;
};

// Subscribes to a gameplay event via member function with weak object safety
template <typename TOwner>
void UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(FGameplayTag MessageTag, TOwner* Object, void (TOwner::*Function)(const FGameplayEventData&))
{
	TWeakObjectPtr<TOwner> WeakObject(Object);
	CallOrStartListeningForGlobalMessage(MessageTag, Object, [WeakObject, Function](const FGameplayEventData& Payload)
	{
		if (TOwner* StrongObject = WeakObject.Get())
		{
			(StrongObject->*Function)(Payload);
		}
	});
}
