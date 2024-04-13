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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static UGlobalEventsSubsystem* GetGlobalEventsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Game States
	 * - BIND_AND_CALL_ON_GAME_STATE_CHANGED - called when the current game state was changed.
	 * - BIND_ON_GAME_STATE_CREATED - called when the game state actor was created. 
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);

	/** Called when the current game state was changed. Code usage example:
	 * BIND_AND_CALL_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGameStateChanged OnGameStateChanged;

	/*********************************************************************************************
	 * On Character Ready
	 * - BIND_AND_CALL_ON_CHARACTER_READY - when character was spawned, possessed and replicated.
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterReady, class APlayerCharacter*, Character, int32, CharacterID);

	/** Called when any character was spawned, possessed and replicated. Code usage example:
	 * BIND_AND_CALL_ON_CHARACTER_READY(this, ThisClass::OnCharacterReady, 0);
	 * Is not BlueprintCallable since should be broadcasted only by OnCharactersReadyHandler.
	 * @warning Don't try to broadcast it in code, instead call OnCharactersReadyHandler's methods. */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++")
	FOnCharacterReady OnCharacterReady;

	/** Internal structure to handle character ready event. */
	FOnCharactersReadyHandler OnCharactersReadyHandler;

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
#define BIND_AND_CALL_ON_GAME_STATE_CHANGED(Obj, Function) \
{ \
	UGlobalEventsSubsystem::Get().OnGameStateChanged.AddUniqueDynamic(Obj, &Function); \
	if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::Menu) \
	{ \
		Obj->Function(ECurrentGameState::Menu); \
	} \
}

/** Helper macro to bind and call the function when any character was spawned and possessed. */
#define BIND_AND_CALL_ON_CHARACTER_READY(Obj, Function, CharacterID) \
{ \
	UGlobalEventsSubsystem::Get().OnCharacterReady.AddUniqueDynamic(Obj, &Function); \
	APlayerCharacter* Character = UMyBlueprintFunctionLibrary::GetPlayerCharacter(CharacterID); \
	if (UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.IsCharacterReady(Character)) \
	{ \
		Obj->Function(Character, CharacterID); \
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