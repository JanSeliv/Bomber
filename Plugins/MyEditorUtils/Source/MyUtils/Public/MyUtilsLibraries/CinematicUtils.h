// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "CinematicUtils.generated.h"

class UMovieSceneSequence;
class UMovieSceneSection;

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
	static const UMovieSceneSequence* FindSubsequence(int32 SubsequenceIndex, const UMovieSceneSequence* MasterSequence);

	/** Returns the length of by given subsequence index or -1 if not found.
	 * @param LevelSequence The sequence to get length of. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetSequenceTotalFrames(const UMovieSceneSequence* LevelSequence);

	/** Finds the first Camera Component inside the specified Level sequence player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UCameraComponent* FindSequenceCameraComponent(class UMovieSceneSequencePlayer* LevelSequencePlayer);

	/** Returns all sections of specified class from the Master sequence in sorted order (from the beginning to the end).
	 * @param MasterSequence The sequence to get sections from.
	 * @param SectionClass The class of sections.
	 * @param OutSections The array to fill with found sections. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static void GetAllSectionsByClass(const UMovieSceneSequence* MasterSequence, TSubclassOf<UMovieSceneSection> SectionClass, TArray<UMovieSceneSection*>& OutSections);

	/** Alternative version of GetAllSectionsByClass with auto cast array to the specified class. */
	template <typename T = UMovieSceneSection>
	static void GetAllSectionsByClass(const UMovieSceneSequence* MasterSequence, TArray<T*>& OutSections);

	/** Resets the sequence player to the beginning.
	 * @param LevelSequencePlayer The sequence player to reset.
	 * @param bKeepCamera If true, camera will not be reset, might be useful when disable the sequence in background without affecting the camera. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void ResetSequence(class UMovieSceneSequencePlayer* LevelSequencePlayer, bool bKeepCamera = false);
};

// Alternative version of GetAllSectionsByClass with auto cast array to the specified class
template <typename T>
void UCinematicUtils::GetAllSectionsByClass(const UMovieSceneSequence* MasterSequence, TArray<T*>& OutSections)
{
	TArray<UMovieSceneSection*> Sections;
	GetAllSectionsByClass(MasterSequence, T::StaticClass(), /*out*/Sections);
	for (UMovieSceneSection* It : Sections)
	{
		OutSections.Emplace(CastChecked<T>(It));
	}
}
