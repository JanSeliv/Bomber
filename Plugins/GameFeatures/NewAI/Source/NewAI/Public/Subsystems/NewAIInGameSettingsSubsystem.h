// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"
//---
#include "NewAIInGameSettingsSubsystem.generated.h"

/**
 * Contains New AI settings that are tweaked by player in Settings menu during the game.
 */
UCLASS(BlueprintType, Blueprintable, Config = "NewAI", DefaultConfig)
class NEWAI_API UNewAIInGameSettingsSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNewAIInGameSettingsSubsystem& Get();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewAIDifficultyChanged, int32, NewDifficultyLevel);

	/** Called when new difficulty level is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnNewAIDifficultyChanged OnNewAIDifficultyChanged;

	/** Returns true is setting enabled to skips previously seen cinematics automatically. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetDifficultyLevel() const { return DifficultyLevelInternal; }

	/** Set new difficulty level. Higher value bigger difficulty. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetDifficultyLevel(int32 InLevel);

protected:
	/** When setting enabled, skips previously seen cinematics automatically.
	 * Is config property, can be set in Settings menu.
	 * @warning in multiplayer, this setting is ignored, so cinematics are always skipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Difficulty Level"))
	int32 DifficultyLevelInternal;
};
