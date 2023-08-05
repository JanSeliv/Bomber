// Copyright (c) Yevhenii Selivanov

#include "Components/NewMainMenuSpotComponent.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NewMainMenuDataAsset.h"
#include "Data/NewMainMenuSubsystem.h"
#include "Data/NewMainMenuTypes.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyDataTable/MyDataTable.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "Sections/MovieSceneSubSection.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuSpotComponent)

// Default constructor
UNewMainMenuSpotComponent::UNewMainMenuSpotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns true if this spot is currently active and possessed by player
bool UNewMainMenuSpotComponent::IsActiveSpot() const
{
	return UNewMainMenuSubsystem::Get().GetActiveMainMenuSpotComponent() == this;
}

// Returns the Skeletal Mesh of the Bomber character
UMySkeletalMeshComponent* UNewMainMenuSpotComponent::GetMySkeletalMeshComponent() const
{
	return GetOwner()->FindComponentByClass<UMySkeletalMeshComponent>();
}

UMySkeletalMeshComponent& UNewMainMenuSpotComponent::GetMeshChecked() const
{
	UMySkeletalMeshComponent* Mesh = GetMySkeletalMeshComponent();
	checkf(Mesh, TEXT("'Mesh' is nullptr, can not get mesh for '%s' spot."), *GetNameSafe(this));
	return *Mesh;
}

// Returns main cinematic of this spot
const ULevelSequence* UNewMainMenuSpotComponent::GetMasterSequence() const
{
	return MasterPlayerInternal ? Cast<ULevelSequence>(MasterPlayerInternal->GetSequence()) : nullptr;
}

// Finds subsequence of this spot by given index
const ULevelSequence* UNewMainMenuSpotComponent::FindSubsequence(int32 SubsequenceIndex) const
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
int32 UNewMainMenuSpotComponent::GetSequenceTotalFrames(const ULevelSequence* LevelSequence)
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
void UNewMainMenuSpotComponent::StopMasterSequence()
{
	if (MasterPlayerInternal)
	{
		MasterPlayerInternal->Stop();
	}
}

// Overridable native event for when play begins for this actor.
void UNewMainMenuSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	UNewMainMenuSubsystem::Get().AddNewMainMenuSpot(this);

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
void UNewMainMenuSpotComponent::OnGameStateChanged(ECurrentGameState CurrentGameState)
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
void UNewMainMenuSpotComponent::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	// Handle current game state if initialized with delay
	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
}

// Obtains and caches cinematic data from the table to this spot
void UNewMainMenuSpotComponent::UpdateCinematicData()
{
	const UDataTable* CinematicsDataTable = UNewMainMenuDataAsset::Get().GetCinematicsDataTable();
	if (!ensureMsgf(CinematicsDataTable, TEXT("'CinematicsDataTable' is nullptr, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	const FPlayerTag& PlayerTag = GetMeshChecked().GetPlayerTag();

	int32 RowIndex = 0;
	TMap<FName, FCinematicRow> CinematicsRows;
	UMyDataTable::GetRows(*CinematicsDataTable, CinematicsRows);
	for (const TTuple<FName, FCinematicRow>& RowIt : CinematicsRows)
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
void UNewMainMenuSpotComponent::CreateMasterSequencePlayer()
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
}

// Plays idle part in loop of current Master Sequence
void UNewMainMenuSpotComponent::PlayIdlePart()
{
	if (!IsActiveSpot())
	{
		// Don't play for inactive spot
		return;
	}

	checkf(MasterPlayerInternal, TEXT("ERROR: 'MasterPlayerInternal' is null!"));

	constexpr int32 IdleSectionIdx = 0;
	const int32 TotalFrames = GetSequenceTotalFrames(FindSubsequence(IdleSectionIdx));

	constexpr int32 FirstFrame = 0;
	MasterPlayerInternal->SetFrameRange(FirstFrame, TotalFrames);
	MasterPlayerInternal->PlayLooping();
}

// Plays main part of current Master Sequence
void UNewMainMenuSpotComponent::PlayMainPart()
{
	if (!IsActiveSpot())
	{
		// Don't play for inactive spot
		return;
	}

	// Change the range back to normal, so the idle will transit to main part
	constexpr int32 FirstFrame = 0;
	const int32 TotalFrames = GetSequenceTotalFrames(GetMasterSequence());
	MasterPlayerInternal->SetFrameRange(FirstFrame, TotalFrames);
}
