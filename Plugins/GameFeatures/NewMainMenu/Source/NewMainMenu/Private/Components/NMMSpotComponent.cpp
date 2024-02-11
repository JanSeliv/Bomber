// Copyright (c) Yevhenii Selivanov

#include "Components/NMMSpotComponent.h"
//---
#include "Bomber.h"
#include "NMMUtils.h"
#include "Components/MyCameraComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "MyDataTable/MyDataTable.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "LevelSequencePlayer.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
//---
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
	return UNMMSpotsSubsystem::Get().GetActiveMainMenuSpotComponent() == this;
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

// Prevents the spot from playing any cinematic
void UNMMSpotComponent::StopMasterSequence()
{
	SetCinematicByState(ENMMState::None);
}

// Activate given cinematic state on this spot
void UNMMSpotComponent::SetCinematicByState(ENMMState MainMenuState)
{
	if (!IsActiveSpot()) // Don't play for inactive spot
	{
		return;
	}

	// --- Load cinematic synchronously if not loaded yet
	const bool bIsCinematicLoading = !MasterPlayerInternal || CinematicRowInternal.LevelSequence.IsPending();
	if (bIsCinematicLoading)
	{
		OnMasterSequenceLoaded(CinematicRowInternal.LevelSequence.LoadSynchronous());
	}
	checkf(MasterPlayerInternal, TEXT("ERROR: [%i] %s:\n'MasterPlayerInternal' is null!"), __LINE__, *FString(__FUNCTION__));

	// --- Set the length of the cinematic
	constexpr int32 FirstFrame = 0;
	const int32 TotalFrames = UNMMUtils::GetCinematicTotalFrames(MainMenuState, MasterPlayerInternal);
	MasterPlayerInternal->SetFrameRange(FirstFrame, TotalFrames);

	// --- Set the playback settings
	const FMovieSceneSequencePlaybackSettings& PlaybackSettings = UNMMUtils::GetCinematicSettings(MainMenuState);
	MasterPlayerInternal->SetPlaybackSettings(PlaybackSettings);
	if (PlaybackSettings.bRestoreState)
	{
		// Reset all 'Keep States' tracks to default
		MasterPlayerInternal->RestorePreAnimatedState();
		MasterPlayerInternal->PreAnimatedState.EnableGlobalPreAnimatedStateCapture();
	}

	// --- Set the playback position
	const FMovieSceneSequencePlaybackParams PlaybackPositionParams = UNMMUtils::GetPlaybackPositionParams(MainMenuState, MasterPlayerInternal);
	MasterPlayerInternal->SetPlaybackPosition(PlaybackPositionParams);

	// --- Play the cinematic (in case of stop it will be paused automatically)
	if (MainMenuState != ENMMState::None)
	{
		MasterPlayerInternal->Play();
	}

	// --- Change the camera according to the cinematic state
	PossessCamera(MainMenuState);

	// --- Update cinematic state, so we could track it
	CinematicStateInternal = MainMenuState;
}

// Overridable native event for when play begins for this actor.
void UNMMSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld()->bIsTearingDown)
	{
		// Don't process modular if world is restarting
		// It could happen since module could be loaded very late, right after request of restarting a level
		return;
	}

	UNMMSpotsSubsystem::Get().AddNewMainMenuSpot(this);

	UpdateCinematicData();
	LoadMasterSequencePlayer();

	// Listen Main Menu states
	UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();
	BaseSubsystem.OnMainMenuStateChanged.AddUniqueDynamic(this, &ThisClass::OnNewMainMenuStateChanged);
	if (BaseSubsystem.GetCurrentMenuState() != ENMMState::None)
	{
		// State is already set, apply it
		OnNewMainMenuStateChanged(BaseSubsystem.GetCurrentMenuState());
	}
}

// Clears all transient data created by this component
void UNMMSpotComponent::OnUnregister()
{
	CinematicRowInternal = FNMMCinematicRow::Empty;

	// Kill current cinematic player
	if (MasterPlayerInternal)
	{
		MasterPlayerInternal->Stop();
		MasterPlayerInternal->ConditionalBeginDestroy();
		MasterPlayerInternal = nullptr;
	}

	if (UNMMSpotsSubsystem* Subsystem = UNMMUtils::GetSpotsSubsystem(this))
	{
		Subsystem->RemoveMainMenuSpot(this);
	}

	Super::OnUnregister();
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
void UNMMSpotComponent::LoadMasterSequencePlayer()
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

	if (FoundMasterSequence.IsValid())
	{
		OnMasterSequenceLoaded(FoundMasterSequence);
	}
	else
	{
		const TAsyncLoadPriority Priority = IsActiveSpot() ? FStreamableManager::AsyncLoadHighPriority : FStreamableManager::DefaultAsyncLoadPriority;
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		StreamableManager.RequestAsyncLoad(FoundMasterSequence.ToSoftObjectPath(),
		                                   FStreamableDelegate::CreateUObject(this, &ThisClass::OnMasterSequenceLoaded, FoundMasterSequence),
		                                   Priority);
	}
}

// Starts viewing through camera of current cinematic
void UNMMSpotComponent::PossessCamera(ENMMState MainMenuState)
{
	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC
		|| !IsActiveSpot()
		|| !ensureMsgf(MasterPlayerInternal, TEXT("ASSERT: [%i] %s:\n'MasterPlayerInternal' is null!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const UCameraComponent* ActiveCamera = nullptr;
	switch (MainMenuState)
	{
	case ENMMState::None:
		ActiveCamera = UMyBlueprintFunctionLibrary::GetLevelCamera();
		break;
	case ENMMState::Idle:
		ActiveCamera = UCinematicUtils::FindSequenceCameraComponent(MasterPlayerInternal);
		break;
	default: break;
	}

	if (ActiveCamera)
	{
		MyPC->SetViewTarget(ActiveCamera->GetOwner());
	}
}

// Marks own cinematic as seen by player for the save system
void UNMMSpotComponent::TryMarkCinematicAsSeen()
{
	if (!IsActiveSpot())
	{
		// Since there are multiple spots, only current one should mark cinematic as seen
		return;
	}

	if (UNMMSaveGameData* SaveGameData = UNMMUtils::GetSaveGameData())
	{
		SaveGameData->MarkCinematicAsSeen(CinematicRowInternal.RowIndex);
	}
}

// Is called when the cinematic was loaded to finish creation
void UNMMSpotComponent::OnMasterSequenceLoaded(TSoftObjectPtr<ULevelSequence> LoadedMasterSequence)
{
	if (MasterPlayerInternal)
	{
		// Is already initialized
		return;
	}

	// Create and cache the master sequence
	ALevelSequenceActor* OutActor = nullptr;
	MasterPlayerInternal = ULevelSequencePlayer::CreateLevelSequencePlayer(this, LoadedMasterSequence.Get(), {}, OutActor);
	checkf(MasterPlayerInternal, TEXT("ERROR: 'MasterPlayerInternal' was not created, something went wrong!"));

	// Override the aspect ratio of the cinematic to the aspect ratio of the screen
	FLevelSequenceCameraSettings CameraSettings;
	CameraSettings.bOverrideAspectRatioAxisConstraint = true;
	CameraSettings.AspectRatioAxisConstraint = UUtilsLibrary::GetViewportAspectRatioAxisConstraint();
	MasterPlayerInternal->Initialize(GetMasterSequence(), GetWorld()->PersistentLevel, CameraSettings);

	// Notify that the spot is ready and finished loading
	UNMMSpotsSubsystem::Get().OnMainMenuSpotReady.Broadcast(this);

	// Bind to react on cinematic finished, is pause instead of stop because of Settings.bPauseAtEnd
	MasterPlayerInternal->OnPause.AddUniqueDynamic(this, &ThisClass::OnMasterSequencePaused);
}

// Called wen the Main Menu state was changed
void UNMMSpotComponent::OnNewMainMenuStateChanged_Implementation(ENMMState NewState)
{
	switch (NewState)
	{
	case ENMMState::Cinematic:
		{
			TryMarkCinematicAsSeen();
			break;
		}
	default: break;
	}

	SetCinematicByState(NewState);
}

// Called when the sequence is paused or when cinematic was ended
void UNMMSpotComponent::OnMasterSequencePaused_Implementation()
{
	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC
		|| CinematicStateInternal != ENMMState::Cinematic)
	{
		// Don't handle if not playing Main Part or is not local player
		return;
	}

	const FFrameNumber CurrentFrame = MasterPlayerInternal->GetCurrentTime().Time.FrameNumber;
	const FFrameNumber EndFrame(UCinematicUtils::GetSequenceTotalFrames(GetMasterSequence()) - 1);
	if (CurrentFrame >= EndFrame)
	{
		// Cinematic is finished, start the countdown
		MyPC->ServerSetGameState(ECurrentGameState::GameStarting);

		CinematicStateInternal = ENMMState::None;
	}
}
