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
	const UMovieScene* InMovieScene = MasterSequence ? MasterSequence->GetMovieScene() : nullptr;
	if (!InMovieScene)
	{
		return nullptr;
	}

	int32 CurrentSubsequenceIdx = 0;
	const TArray<UMovieSceneSection*>& AllSections = InMovieScene->GetAllSections();
	for (int32 Idx = AllSections.Num() - 1; Idx >= 0; --Idx)
	{
		const UMovieSceneSubSection* SubSection = Cast<UMovieSceneSubSection>(AllSections[Idx]);
		const UMovieSceneSequence* SubSequence = SubSection ? Cast<UMovieSceneSequence>(SubSection->GetSequence()) : nullptr;
		if (!SubSection)
		{
			continue;
		}

		if (CurrentSubsequenceIdx == SubsequenceIndex)
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
	if (!InMovieScene)
	{
		return nullptr;
	}

	const TArray<UMovieSceneSection*>& AllSections = InMovieScene->GetAllSections();
	for (const UMovieSceneSection* Section : AllSections)
	{
		const UMovieSceneCameraCutSection* CameraCutSection = Cast<UMovieSceneCameraCutSection>(Section);
		if (!CameraCutSection)
		{
			continue;
		}

		static const FMovieSceneSequenceID ID{0};
		IMovieScenePlayer& InPlayer = *LevelSequencePlayer;
		if (UCameraComponent* FoundCamera = CameraCutSection->GetFirstCamera(InPlayer, ID))
		{
			return FoundCamera;
		}
	}

	return nullptr;
}
