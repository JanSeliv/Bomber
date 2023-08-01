// Copyright (c) Yevhenii Selivanov

#include "NewMainMenuSpotComponent.h"
//---
#include "Controllers/MyPlayerController.h"
#include "Data/NewMainMenuDataAsset.h"
#include "Data/NewMainMenuSubsystem.h"
#include "Data/NewMainMenuTypes.h"
#include "MyDataTable/MyDataTable.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "Camera/CameraActor.h"
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

void UNewMainMenuSpotComponent::SetCameraViewOnSpot(bool bBlend)
{
	AMyPlayerController* PC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!PC || !CameraActorInternal)
	{
		return;
	}

	const float BlendTime = bBlend ? 0.5f : 0.0f;
	PC->SetViewTargetWithBlend(CameraActorInternal, BlendTime);
}

// Overridable native event for when play begins for this actor.
void UNewMainMenuSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	UNewMainMenuSubsystem::Get().AddNewMainMenuSpot(this);

	LoadMasterSequence();
	TrySetCameraViewByDefault();
}

// Sets camera view to this spot if current level type is equal to the spot's player
void UNewMainMenuSpotComponent::TrySetCameraViewByDefault()
{
	const ELevelType CurrentLevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	const ELevelType PlayerByLevelType = GetMeshChecked().GetAssociatedLevelType();
	const bool bCanReadLevelType = CurrentLevelType != ELT::None && PlayerByLevelType != ELT::None;
	if (ensureMsgf(bCanReadLevelType, TEXT("'bCanReadLevelType' condition is FALSE, can not determine the level type for '%s' spot."), *GetNameSafe(this))
		&& CurrentLevelType == PlayerByLevelType)
	{
		constexpr bool bBlend = false;
		SetCameraViewOnSpot(bBlend);
	}
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
	MasterSequenceInternal = LoadedMasterSequence.Get();
	PlayLoopIdle();
}

void UNewMainMenuSpotComponent::PlayLoopIdle()
{
	if (!ensureMsgf(MasterSequenceInternal, TEXT("'MasterSequenceInternal' is nullptr, can not play idle for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	// Find Idle Sequence: is a first sub sequence of the master sequence
	ULevelSequence* IdleSequence = nullptr;
	const UMovieScene* InMovieScene = MasterSequenceInternal->GetMovieScene();
	checkf(InMovieScene, TEXT("'InMovieScene' is nullptr, can not play idle for '%s' spot."), *GetNameSafe(this));
	for (const UMovieSceneSection* SectionIt : InMovieScene->GetAllSections())
	{
		if (const UMovieSceneSubSection* SubSection = Cast<UMovieSceneSubSection>(SectionIt))
		{
			IdleSequence = CastChecked<ULevelSequence>(SubSection->GetSequence());
			UE_LOG(LogTemp, Log, TEXT("IdleSequence: %s"), *GetNameSafe(IdleSequence));
			break;
		}
	}

	if (!ensureMsgf(IdleSequence, TEXT("'IdleSequence' is nullptr, can not play idle for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	// Create and cache the idle sequence
	ALevelSequenceActor* OutActor = nullptr;
	IdlePlayerInternal = ULevelSequencePlayer::CreateLevelSequencePlayer(this, IdleSequence, {}, OutActor);
	IdlePlayerInternal->PlayLooping();
}
