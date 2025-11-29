// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"

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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetTickInterval() const { return TickInternal; }

	/** Return the summary time required to start the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetStartingCountdown() const { return StartingCountdownInternal; }

	/** Returns the left second to the end of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetInGameCountdown() const { return InGameCountdownInternal; }

	/** Returns the empty startup map that is used only in builds at launch and designed for fast boot before transitioning to the gameplay level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE TSoftObjectPtr<UWorld>& GetStartupLevel() const { return StartupLevelInternal; }

	/** Returns the main gameplay level to load. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE TSoftObjectPtr<UWorld>& GetMainLevel() const { return MainLevelInternal; }

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

	/* Empty startup map that is used only in builds at launch.
	 * Designed for fast boot before transitioning to the gameplay level.
	 * Contains no references to content to ensure minimal load time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "Empty Startup Map"))
	TSoftObjectPtr<UWorld> StartupLevelInternal = nullptr;

	/** The main gameplay level to load. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "Main Level"))
	TSoftObjectPtr<UWorld> MainLevelInternal = nullptr;
};