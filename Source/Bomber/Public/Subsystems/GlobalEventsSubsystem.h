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

	/*********************************************************************************************
	 * Game States
	 ********************************************************************************************* */
public:
	/** Returns the pointer to this Subsystem. Code usage example:
	 * BIND_AND_CALL_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static UGlobalEventsSubsystem* GetGlobalEventsSubsystem(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);

	/** Called when the current game state was changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGameStateChanged OnGameStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndGameStateChanged, EEndGameState, EndGameState);

	/** Called when player's match result was changed (Win, lose, draw or none applied). */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnEndGameStateChanged OnEndGameStateChanged;

	/*********************************************************************************************
	 * On Character Ready
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

/** Helper macro to bind and call the function when the game state was changed. */
#define BIND_AND_CALL_ON_GAME_STATE_CHANGED(Obj, Function) \
{ \
	UGlobalEventsSubsystem::Get().OnGameStateChanged.AddUniqueDynamic(Obj, &Function); \
	if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::Menu) \
	{ \
		Function(ECurrentGameState::Menu); \
	} \
}

/** Helper macro to bind and call the function when the local player character was spawned and possessed. */
#define BIND_AND_CALL_ON_CHARACTER_READY(Obj, Function, CharacterID) \
{ \
	UGlobalEventsSubsystem::Get().OnCharacterReady.AddUniqueDynamic(Obj, &Function); \
	APlayerCharacter* Character = UMyBlueprintFunctionLibrary::GetPlayerCharacter(CharacterID); \
	if (UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.IsCharacterReady(Character)) \
	{ \
		Function(Character, CharacterID); \
	} \
}