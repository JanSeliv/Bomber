// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "GlobalEventsSubsystem.generated.h"

/** Helper macro to bind and call the function when the game state was changed. */
#define BIND_AND_CALL_ON_GAME_STATE_CHANGED(Obj, Function) \
{ \
	UGlobalEventsSubsystem::Get().OnGameStateChanged.AddUniqueDynamic(Obj, &Function); \
	if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::Menu) \
	{ \
		Function(ECurrentGameState::Menu); \
	} \
}

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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static UGlobalEventsSubsystem* GetGlobalEventsSubsystem(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);

	/** Called when the current game state was changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGameStateChanged OnGameStateChanged;
};