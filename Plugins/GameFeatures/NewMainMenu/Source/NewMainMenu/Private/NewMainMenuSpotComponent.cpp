// Copyright (c) Yevhenii Selivanov

#include "NewMainMenuSpotComponent.h"
//---
#include "Data/NewMainMenuDataAsset.h"
#include "Data/NewMainMenuSubsystem.h"
#include "Data/NewMainMenuTypes.h"
#include "MyDataTable/MyDataTable.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Sections/MovieSceneSubSection.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuSpotComponent)

// Guarantees that the data asset is loaded, otherwise, it will crash
const UNewMainMenuDataAsset& UNewMainMenuSpotComponent::GetNewMainMenuDataAssetChecked() const
{
	const UNewMainMenuDataAsset* NewMainMenuDataAsset = GetNewMainMenuDataAsset();
	checkf(NewMainMenuDataAsset, TEXT("%s: 'NewMainMenuDataAssetInternal' is not set"), *FString(__FUNCTION__));
	return *NewMainMenuDataAsset;
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

	LoadMasterSequence();
}

// Loads cinematic of this spot
void UNewMainMenuSpotComponent::LoadMasterSequence()
{
	const UDataTable* CinematicsDataTable = GetNewMainMenuDataAssetChecked().GetCinematicsDataTable();
	if (!ensureMsgf(CinematicsDataTable, TEXT("'CinematicsDataTable' is nullptr, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	TSoftObjectPtr<ULevelSequence> LevelSequenceToLoad = nullptr;

	const FPlayerTag& PlayerTag = GetMeshChecked().GetPlayerTag();
	TMap<FName, FCinematicRow> CinematicsRows;
	UMyDataTable::GetRows(*CinematicsDataTable, CinematicsRows);
	for (const TTuple<FName, FCinematicRow>& RowIt : CinematicsRows)
	{
		if (RowIt.Value.PlayerTag == PlayerTag)
		{
			LevelSequenceToLoad = RowIt.Value.LevelSequence;
			break;
		}
	}

	if (!ensureMsgf(!LevelSequenceToLoad.IsNull(), TEXT("'LevelSequenceToLoad' is not found, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	if (LevelSequenceToLoad.IsValid())
	{
		OnMasterSequenceLoaded(LevelSequenceToLoad);
	}
	else
	{
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		StreamableManager.RequestAsyncLoad(LevelSequenceToLoad.ToSoftObjectPath(),
		                                   FStreamableDelegate::CreateUObject(this, &UNewMainMenuSpotComponent::OnMasterSequenceLoaded, LevelSequenceToLoad));
	}
}

void UNewMainMenuSpotComponent::OnMasterSequenceLoaded(TSoftObjectPtr<ULevelSequence> LoadedMasterSequence)
{
	// Create and cache the master sequence
	ALevelSequenceActor* OutActor = nullptr;
	MasterPlayerInternal = ULevelSequencePlayer::CreateLevelSequencePlayer(this, LoadedMasterSequence.Get(), {}, OutActor);

	if (IsActiveSpot())
	{
		PlayLoopIdle();
	}
}

// Plays idle in loop of this spot
void UNewMainMenuSpotComponent::PlayLoopIdle()
{
	checkf(MasterPlayerInternal, TEXT("ERROR: 'MasterPlayerInternal' is null!"));

	constexpr int32 IdleSectionIdx = 0;
	const int32 TotalFrames = GetSubsequenceTotalFrames(IdleSectionIdx);

	constexpr int32 FirstFrame = 0;
	MasterPlayerInternal->SetFrameRange(FirstFrame, TotalFrames);
	MasterPlayerInternal->PlayLooping();
}
