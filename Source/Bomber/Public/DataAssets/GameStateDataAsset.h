// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"
//---
#include "GameStateDataAsset.generated.h"

/**
 * The data of the game match.
 */
UCLASS()
class BOMBER_API UGameStateDataAsset final : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the Game State data asset. */
	static const UGameStateDataAsset& Get();

	/** Returns general value how ofter update actors and states in the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetTickInterval() const { return TickInternal; }

	/** Return the summary time required to start the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetStartingCountdown() const { return StartingCountdownInternal; }

	/** Returns the left second to the end of the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetInGameCountdown() const { return InGameCountdownInternal; }

	/** Assign new match duration.
	 * Might be useful for testing, but shouldn't be used in shipping. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void SetInGameCountdown(int32 NewInGameCountdown) { InGameCountdownInternal = NewInGameCountdown; }

protected:
	/** General value how ofter update actors and states in the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Tick Interval", ShowOnlyInnerProperties))
	float TickInternal = 0.2f;

	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "Starting Countdown"))
	int32 StartingCountdownInternal = 3;

	/** Seconds to the end of the round. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "In-Game Countdown"))
	int32 InGameCountdownInternal = 120;
};