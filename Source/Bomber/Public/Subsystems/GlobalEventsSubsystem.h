// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
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

	/** Returns the pointer to this Subsystem. Code usage example:
	 * BIND_AND_CALL_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static UGlobalEventsSubsystem* GetGlobalEventsSubsystem(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);

	/** Called when the current game state was changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGameStateChanged OnGameStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterWithIDPossessed, class APlayerCharacter*, Character, int32, CharacterID);

	/** Called when any character was spawned and possessed. Code usage example:
	 * BIND_ON_CHARACTER_WITH_ID_POSSESSED(this, ThisClass::OnCharacterWithIDPossessed, 0); */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnCharacterWithIDPossessed OnCharacterWithIDPossessed;
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
#define BIND_ON_CHARACTER_WITH_ID_POSSESSED(Obj, Function, CharacterID) \
{ \
	UGlobalEventsSubsystem::Get().OnCharacterWithIDPossessed.AddUniqueDynamic(Obj, &Function); \
	if (APlayerCharacter* PlayerCharacter = UMyBlueprintFunctionLibrary::GetPlayerCharacter(CharacterID)) \
	{ \
		Function(PlayerCharacter, CharacterID); \
	} \
}