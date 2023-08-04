// Copyright (c) Yevhenii Selivanov

#include "Components/NewMainMenuSpotComponent.h"
//---
#include "Bomber.h"
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
	const ELevelType CurrentLevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	const ELevelType PlayerByLevelType = GetMeshChecked().GetAssociatedLevelType();
	const bool bCanReadLevelType = CurrentLevelType != ELT::None && PlayerByLevelType != ELT::None;
	return ensureMsgf(bCanReadLevelType, TEXT("'bCanReadLevelType' condition is FALSE, can not determine the level type for '%s' spot."), *GetNameSafe(this))
		&& CurrentLevelType == PlayerByLevelType;
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
int32 UNewMainMenuSpotComponent::GetSubsequenceTotalFrames(int32 SubsequenceIndex) const
{
	const ULevelSequence* IdleSubsequence = FindSubsequence(SubsequenceIndex);
	if (!ensureMsgf(IdleSubsequence, TEXT("'IdleSequence' is nullptr, can not play idle for '%s' spot."), *GetNameSafe(this)))
	{
		return INDEX_NONE;
	}

	const UMovieScene* MovieScene = IdleSubsequence ? IdleSubsequence->GetMovieScene() : nullptr;
	const FFrameRate TickResolution = MovieScene->GetTickResolution();
	const FFrameRate DisplayRate = MovieScene->GetDisplayRate();

	const TRange<FFrameNumber> SubSectionRange = MovieScene->GetPlaybackRange();
	const FFrameNumber SrcStartFrame = UE::MovieScene::DiscreteInclusiveLower(SubSectionRange);
	const FFrameNumber SrcEndFrame = UE::MovieScene::DiscreteExclusiveUpper(SubSectionRange);

	const FFrameTime StartFrameTime = ConvertFrameTime(SrcStartFrame, TickResolution, DisplayRate);
	const FFrameTime EndFrameTime = ConvertFrameTime(SrcEndFrame, TickResolution, DisplayRate);
	return (EndFrameTime.FloorToFrame() - StartFrameTime.FloorToFrame()).Value;
}

// Overridable native event for when play begins for this actor.
void UNewMainMenuSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	UNewMainMenuSubsystem::Get().AddNewMainMenuSpot(this);

	CreateMasterSequencePlayer();

	AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	checkf(MyGameState, TEXT("%s: 'MyGameState' is null"), *FString(__FUNCTION__));

	// Handle current game state if initialized with delay
	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
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

// Loads cinematic of this spot
void UNewMainMenuSpotComponent::CreateMasterSequencePlayer()
{
	if (MasterPlayerInternal)
	{
		// Is already created
		return;
	}

	const UDataTable* CinematicsDataTable = UNewMainMenuDataAsset::Get().GetCinematicsDataTable();
	if (!ensureMsgf(CinematicsDataTable, TEXT("'CinematicsDataTable' is nullptr, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	TSoftObjectPtr<ULevelSequence> FoundMasterSequence = nullptr;

	const FPlayerTag& PlayerTag = GetMeshChecked().GetPlayerTag();
	TMap<FName, FCinematicRow> CinematicsRows;
	UMyDataTable::GetRows(*CinematicsDataTable, CinematicsRows);
	for (const TTuple<FName, FCinematicRow>& RowIt : CinematicsRows)
	{
		if (RowIt.Value.PlayerTag == PlayerTag)
		{
			FoundMasterSequence = RowIt.Value.LevelSequence;
			break;
		}
	}

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
	const int32 TotalFrames = GetSubsequenceTotalFrames(IdleSectionIdx);

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

	// @TODO
	UE_LOG(LogTemp, Warning, TEXT("--- PLAYING MAIN PART ---"));
}
