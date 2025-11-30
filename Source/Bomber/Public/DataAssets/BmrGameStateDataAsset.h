// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"

#include "BmrGameStateDataAsset.generated.h"

/**
 * The data of the game match.
 */
UCLASS()
class BOMBER_API UBmrGameStateDataAsset final : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the Game State data asset. */
	static const UBmrGameStateDataAsset& Get();

	/** Returns general value how ofter update actors and states in the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE float GetTickInterval() const { return TickInterval; }

	/** Return the summary time required to start the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetStartingCountdown() const { return StartingCountdown; }

	/** Returns the left second to the end of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetInGameCountdown() const { return InGameCountdown; }

	/** Returns the empty startup map that is used only in builds at launch and designed for fast boot before transitioning to the gameplay level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE TSoftObjectPtr<UWorld>& GetStartupLevel() const { return StartupLevel; }

	/** Returns the main gameplay level to load. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE TSoftObjectPtr<UWorld>& GetMainLevel() const { return MainLevel; }

protected:
	/** General value how ofter update actors and states in the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	float TickInterval = 0.2f;

	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 StartingCountdown = 3;

	/** Seconds to the end of the round. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 InGameCountdown = 120;

	/* Empty startup map that is used only in builds at launch.
	 * Designed for fast boot before transitioning to the gameplay level.
	 * Contains no references to content to ensure minimal load time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<UWorld> StartupLevel = nullptr;

	/** The main gameplay level to load. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<UWorld> MainLevel = nullptr;
};