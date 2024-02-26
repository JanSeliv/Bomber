// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"
//---
#include "GameDifficultySubsystem.generated.h"

/**
 * The type of the game difficulty. 
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EGameDifficulty : uint8
{
	None = 0 UMETA(Hidden),
	///< The easiest difficulty, is 0 as difficulty level
	Easy = 1 << 0,
	///< The normal difficulty, is 1 as difficulty level
	Normal = 1 << 1,
	///< The hardest difficulty, is 2 as difficulty level
	Hard = 1 << 2,
	///< Original game difficulty, where AI is hardcoded in controller, but very smart
	Vanilla = 1 << 3 UMETA(Hidden),
	Any = Easy | Normal | Hard | Vanilla
};

ENUM_CLASS_FLAGS(EGameDifficulty);

/**
 * Contains New AI settings that are tweaked by player in Settings menu during the game.
 */
UCLASS(BlueprintType, Blueprintable, Config = "GameUserSettings", DefaultConfig)
class BOMBER_API UGameDifficultySubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UGameDifficultySubsystem& Get();

	/** Returns the pointer to this Subsystem. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext"))
	static UGameDifficultySubsystem* GetGameDifficultySubsystem(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameDifficultyChanged, int32, NewDifficultyLevel);

	/** Called when new difficulty level is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnGameDifficultyChanged OnGameDifficultyChanged;

	/** Returns current difficulty as enum type, e.g: EGameDifficulty::Easy */
	UFUNCTION(BlueprintPure, Category = "C++")
	EGameDifficulty GetDifficultyType() const;

	/** Sets new game difficulty by enum type. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetDifficultyType(EGameDifficulty InDifficultyType);

	/** Returns true if the game difficulty level is matched with one or more specified types. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool HasDifficulty(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 DifficultiesBitmask) const;

	/** Returns current difficulty level.
	 * Where 0 is the easiest and 3 is the hardest. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetDifficultyLevel() const { return DifficultyLevelInternal; }

	/** Set new difficulty level. Higher value bigger difficulty.
	 * Where 0 is the easiest and 3 is the hardest. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetDifficultyLevel(int32 InLevel);

	/** Applies current difficulty level to the game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ApplyGameDifficulty();

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The game difficulty level, where 0 is the easiest and 3 is the hardest.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Difficulty Level"))
	int32 DifficultyLevelInternal;
};
