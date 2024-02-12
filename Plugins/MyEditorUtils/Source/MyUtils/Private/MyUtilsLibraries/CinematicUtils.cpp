// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/CinematicUtils.h"
//---
#include "MovieScene.h"
#include "MovieSceneSequence.h"
#include "MovieSceneSequencePlayer.h"
#include "Camera/CameraComponent.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "Sections/MovieSceneSubSection.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(CinematicUtils)

// Finds subsequence by given index inside specified Master sequence
const UMovieSceneSequence* UCinematicUtils::FindSubsequence(int32 SubsequenceIndex, const UMovieSceneSequence* MasterSequence)
{
	TArray<UMovieSceneSubSection*> AllTracks;
	GetAllSectionsByClass(MasterSequence, AllTracks);

	int32 CurrentSubsequenceIdx = 0;
	for (const UMovieSceneSubSection* It : AllTracks)
	{
		const UMovieSceneSequence* SubSequence = It ? It->GetSequence() : nullptr;
		if (SubSequence
			&& CurrentSubsequenceIdx == SubsequenceIndex)
		{
			return SubSequence;
		}

		CurrentSubsequenceIdx++;
	}

	return nullptr;
}

// Returns the length of by given subsequence index
int32 UCinematicUtils::GetSequenceTotalFrames(const UMovieSceneSequence* LevelSequence)
{
	if (!ensureMsgf(LevelSequence, TEXT("'LevelSequence' is not valid")))
	{
		return INDEX_NONE;
	}

	const UMovieScene* MovieScene = LevelSequence->GetMovieScene();
	const FFrameRate TickResolution = MovieScene->GetTickResolution();
	const FFrameRate DisplayRate = MovieScene->GetDisplayRate();

	const TRange<FFrameNumber> SubSectionRange = MovieScene->GetPlaybackRange();
	const FFrameNumber SrcStartFrame = UE::MovieScene::DiscreteInclusiveLower(SubSectionRange);
	const FFrameNumber SrcEndFrame = UE::MovieScene::DiscreteExclusiveUpper(SubSectionRange);

	const FFrameTime StartFrameTime = ConvertFrameTime(SrcStartFrame, TickResolution, DisplayRate);
	const FFrameTime EndFrameTime = ConvertFrameTime(SrcEndFrame, TickResolution, DisplayRate);
	return (EndFrameTime.FloorToFrame() - StartFrameTime.FloorToFrame()).Value;
}

// Finds the first Camera Component inside the specified Level sequence
UCameraComponent* UCinematicUtils::FindSequenceCameraComponent(UMovieSceneSequencePlayer* LevelSequencePlayer)
{
	const UMovieSceneSequence* LevelSequence = LevelSequencePlayer ? LevelSequencePlayer->GetSequence() : nullptr;
	const UMovieScene* InMovieScene = LevelSequence ? LevelSequence->GetMovieScene() : nullptr;
	if (!ensureMsgf(InMovieScene, TEXT("ASSERT: [%i] %s:\n'InMovieScene' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	TArray<UMovieSceneCameraCutSection*> AllCameraCutSections;
	GetAllSectionsByClass(LevelSequence, AllCameraCutSections);

	for (const UMovieSceneCameraCutSection* CameraCutSectionIt : AllCameraCutSections)
	{
		checkf(CameraCutSectionIt, TEXT("ERROR: [%i] %s:\n'CameraCutSectionIt' is null!"), __LINE__, *FString(__FUNCTION__));
		static const FMovieSceneSequenceID ID{0};
		IMovieScenePlayer& InPlayer = *LevelSequencePlayer;
		if (UCameraComponent* FoundCamera = CameraCutSectionIt->GetFirstCamera(InPlayer, ID))
		{
			return FoundCamera;
		}
	}

	return nullptr;
}

// Returns all sections of specified class from the Master sequence in sorted order (from the beginning to the end)
void UCinematicUtils::GetAllSectionsByClass(const UMovieSceneSequence* MasterSequence, TSubclassOf<UMovieSceneSection> SectionClass, TArray<UMovieSceneSection*>& OutSections)
{
	if (!OutSections.IsEmpty())
	{
		OutSections.Empty();
	}

	const UMovieScene* InMovieScene = MasterSequence ? MasterSequence->GetMovieScene() : nullptr;
	if (!ensureMsgf(InMovieScene, TEXT("ASSERT: [%i] %s:\n'InMovieScene' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(SectionClass, TEXT("ASSERT: [%i] %s:\n'SectionClass' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Find all sections of specified class
	const TArray<UMovieSceneSection*> AllSections = InMovieScene->GetAllSections();
	for (UMovieSceneSection* Section : AllSections)
	{
		if (Section && Section->IsA(SectionClass))
		{
			OutSections.Emplace(Section);
		}
	}

	if (!ensureMsgf(!OutSections.IsEmpty(), TEXT("ASSERT: [%i] %s:\nIs not found any section of class '%s' in the Master sequence!"), __LINE__, *FString(__FUNCTION__), *SectionClass->GetName()))
	{
		return;
	}

	// Sort all sections by their time
	OutSections.Sort([](const UMovieSceneSection& A, const UMovieSceneSection& B)
	{
		return A.GetRange().GetLowerBoundValue() < B.GetRange().GetLowerBoundValue();
	});
}
