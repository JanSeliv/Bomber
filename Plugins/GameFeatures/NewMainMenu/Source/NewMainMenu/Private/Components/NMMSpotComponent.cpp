// Copyright (c) Yevhenii Selivanov

#include "Components/NMMSpotComponent.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSubsystem.h"
#include "Data/NMMTypes.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyDataTable/MyDataTable.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "Sections/MovieSceneSubSection.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMSpotComponent)

// Default constructor
UNMMSpotComponent::UNMMSpotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns true if this spot is currently active and possessed by player
bool UNMMSpotComponent::IsActiveSpot() const
{
	return UNMMSubsystem::Get().GetActiveMainMenuSpotComponent() == this;
}

// Returns the Skeletal Mesh of the Bomber character
UMySkeletalMeshComponent* UNMMSpotComponent::GetMySkeletalMeshComponent() const
{
	return GetOwner()->FindComponentByClass<UMySkeletalMeshComponent>();
}

UMySkeletalMeshComponent& UNMMSpotComponent::GetMeshChecked() const
{
	UMySkeletalMeshComponent* Mesh = GetMySkeletalMeshComponent();
	checkf(Mesh, TEXT("'Mesh' is nullptr, can not get mesh for '%s' spot."), *GetNameSafe(this));
	return *Mesh;
}

// Returns main cinematic of this spot
ULevelSequence* UNMMSpotComponent::GetMasterSequence() const
{
	return MasterPlayerInternal ? Cast<ULevelSequence>(MasterPlayerInternal->GetSequence()) : nullptr;
}

// Finds subsequence of this spot by given index
const ULevelSequence* UNMMSpotComponent::FindSubsequence(int32 SubsequenceIndex) const
{
	const ULevelSequence* MasterSequence = GetMasterSequence();
	const UMovieScene* InMovieScene = MasterSequence ? MasterSequence->GetMovieScene() : nullptr;
	if (!ensureMsgf(InMovieScene, TEXT("'InMovieScene' is nullptr, can not find subsequence for '%s' spot."), *GetNameSafe(this)))
	{
		return nullptr;
	}

	int32 CurrentSubsequenceIdx = 0;
	const TArray<UMovieSceneSection*>& AllSections = InMovieScene->GetAllSections();
	for (int32 Idx = AllSections.Num() - 1; Idx >= 0; --Idx)
	{
		const UMovieSceneSubSection* SubSection = Cast<UMovieSceneSubSection>(AllSections[Idx]);
		const ULevelSequence* SubSequence = SubSection ? Cast<ULevelSequence>(SubSection->GetSequence()) : nullptr;
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
int32 UNMMSpotComponent::GetSequenceTotalFrames(const ULevelSequence* LevelSequence)
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

// Prevents the spot from playing any cinematic
void UNMMSpotComponent::StopMasterSequence()
{
	if (MasterPlayerInternal)
	{
		MasterPlayerInternal->Stop();
	}
}

// Overridable native event for when play begins for this actor.
void UNMMSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	UNMMSubsystem::Get().AddNewMainMenuSpot(this);

	UpdateCinematicData();
	CreateMasterSequencePlayer();

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		BindOnGameStateChanged(MyGameState);
	}
	else if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController())
	{
		MyPC->OnGameStateCreated.AddUniqueDynamic(this, &ThisClass::BindOnGameStateChanged);
	}
}

// Called when the current game state was changed
void UNMMSpotComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
	case ECurrentGameState::Menu:
		{
			PlayIdlePart();
			break;
		}
	case ECurrentGameState::Cinematic:
		{
			PlayMainPart();
			break;
		}
	default: break;
	}
}

// Is called to start listening game state changes
void UNMMSpotComponent::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	checkf(MyGameState, TEXT("ERROR: 'MyGameState' is null!"));

	// Handle current game state if initialized with delay
	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
}

// Obtains and caches cinematic data from the table to this spot
void UNMMSpotComponent::UpdateCinematicData()
{
	const UDataTable* CinematicsDataTable = UNMMDataAsset::Get().GetCinematicsDataTable();
	if (!ensureMsgf(CinematicsDataTable, TEXT("'CinematicsDataTable' is nullptr, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	const FPlayerTag& PlayerTag = GetMeshChecked().GetPlayerTag();

	int32 RowIndex = 0;
	TMap<FName, FNMMCinematicRow> CinematicsRows;
	UMyDataTable::GetRows(*CinematicsDataTable, CinematicsRows);
	for (const TTuple<FName, FNMMCinematicRow>& RowIt : CinematicsRows)
	{
		if (RowIt.Value.PlayerTag == PlayerTag)
		{
			CinematicRowInternal = RowIt.Value;
			break;
		}
		++RowIndex;
	}

	if (ensureMsgf(!CinematicRowInternal.IsEmpty(), TEXT("%s: 'CinematicRowInternal' is not found for '%s' spot."), *FString(__FUNCTION__), *GetNameSafe(this)))
	{
		CinematicRowInternal.RowIndex = RowIndex;
	}
}

// Loads cinematic of this spot
void UNMMSpotComponent::CreateMasterSequencePlayer()
{
	if (MasterPlayerInternal)
	{
		// Is already created
		return;
	}

	const TSoftObjectPtr<ULevelSequence> FoundMasterSequence = CinematicRowInternal.LevelSequence;
	if (!ensureMsgf(!FoundMasterSequence.IsNull(), TEXT("'LevelSequenceToLoad' is not found, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	// Create and cache the master sequence
	ALevelSequenceActor* OutActor = nullptr;
	MasterPlayerInternal = ULevelSequencePlayer::CreateLevelSequencePlayer(this, FoundMasterSequence.LoadSynchronous(), {}, OutActor);
	checkf(MasterPlayerInternal, TEXT("ERROR: 'MasterPlayerInternal' was not created, something went wrong!"));

	// Override the aspect ratio of the cinematic to the aspect ratio of the screen
	FLevelSequenceCameraSettings Settings;
	Settings.bOverrideAspectRatioAxisConstraint = true;
	Settings.AspectRatioAxisConstraint = UUtilsLibrary::GetViewportAspectRatioAxisConstraint();
	MasterPlayerInternal->Initialize(GetMasterSequence(), GetWorld()->PersistentLevel, Settings);

	// Bind to react on cinematic finished, is pause instead of stop because of Settings.bPauseAtEnd
	MasterPlayerInternal->OnPause.AddUniqueDynamic(this, &ThisClass::OnMasterSequencePaused);
}

// Called when the sequence is paused or when cinematic was ended
void UNMMSpotComponent::OnMasterSequencePaused_Implementation()
{
	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC
		|| AMyGameStateBase::GetCurrentGameState() != ECurrentGameState::Cinematic)
	{
		// Don't handle if not cinematic state or is not local player
		return;
	}

	// Start the countdown
	MyPC->ServerSetGameState(ECurrentGameState::GameStarting);
}

// Plays idle part in loop of current Master Sequence
void UNMMSpotComponent::PlayIdlePart()
{
	if (!IsActiveSpot() // Don't play for inactive spot
		|| !ensureMsgf(MasterPlayerInternal, TEXT("'MasterPlayerInternal' is not valid, player has to be created first!")))
	{
		return;
	}

	constexpr int32 IdleSectionIdx = 0;
	const int32 TotalFrames = GetSequenceTotalFrames(FindSubsequence(IdleSectionIdx));

	constexpr int32 FirstFrame = 0;
	MasterPlayerInternal->SetFrameRange(FirstFrame, TotalFrames);
	MasterPlayerInternal->PlayLooping();
}

// Plays main part of current Master Sequence
void UNMMSpotComponent::PlayMainPart()
{
	if (!IsActiveSpot() // Don't play for inactive spot
		|| !ensureMsgf(MasterPlayerInternal, TEXT("'MasterPlayerInternal' is not valid, player has to be created first!"))
		|| !ensureMsgf(MasterPlayerInternal->IsPlaying(), TEXT("'MasterPlayerInternal' is not playing, idle has to be played first!")))
	{
		return;
	}

	// Change the range back to normal, so the idle will transit to main part
	constexpr int32 FirstFrame = 0;
	const int32 TotalFrames = GetSequenceTotalFrames(GetMasterSequence());
	MasterPlayerInternal->SetFrameRange(FirstFrame, TotalFrames);

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.LoopCount.Value = 0; // Disable looping to play the main part once
	Settings.bPauseAtEnd = true; // Pause at the end to stay at the end position
	MasterPlayerInternal->SetPlaybackSettings(Settings);
}
