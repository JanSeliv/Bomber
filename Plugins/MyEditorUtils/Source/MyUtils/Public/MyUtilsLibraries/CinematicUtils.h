// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "CinematicUtils.generated.h"

class ULevelSequence;

/**
 * The cinematic functions library.
 * Extends Epic's API with some useful functions and tricks.
 */
UCLASS()
class MYUTILS_API UCinematicUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Finds subsequence by given index inside specified Master sequence.
	 * @param SubsequenceIndex In index of subsequence to find.
	 * @param MasterSequence The sequence holder to find subsequence in.
	 * @return The found subsequence or nullptr if not found. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const ULevelSequence* FindSubsequence(int32 SubsequenceIndex, const ULevelSequence* MasterSequence);

	/** Returns the length of by given subsequence index or -1 if not found.
	 * @param LevelSequence The sequence to get length of. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetSequenceTotalFrames(const ULevelSequence* LevelSequence);

	/** Finds the first Camera Component inside the specified Level sequence player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UCameraComponent* FindSequenceCameraComponent(class ULevelSequencePlayer* LevelSequencePlayer);
};
