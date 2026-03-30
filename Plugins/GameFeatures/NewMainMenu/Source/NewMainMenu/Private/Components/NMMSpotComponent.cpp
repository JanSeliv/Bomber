// Copyright (c) Yevhenii Selivanov

#include "Components/NMMSpotComponent.h"

// NMM
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "DalSubsystem.h"
#include "MyUtilsLibraries/AsyncLoadUtilsLibrary.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "LevelSequencePlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMSpotComponent)

// Default constructor
UNMMSpotComponent::UNMMSpotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetActiveFlag(true);
}

// Returns true if this spot is currently active and possessed by player
bool UNMMSpotComponent::IsCurrentSpot() const
{
	return UNMMSpotsSubsystem::Get().GetCurrentSpot() == this;
}

// Returns true if this spot is visible, unlocked and can be selected by player
bool UNMMSpotComponent::IsSpotAvailable() const
{
	const UBmrSkeletalMeshComponent* MeshComponent = GetMeshComponent();
	return IsActive()
	       && MeshComponent
	       && MeshComponent->IsActive()
	       && MeshComponent->IsVisible();
}

// Returns true if this spot current skin is unlocked and can be selected by player
bool UNMMSpotComponent::IsSpotSkinAvailable() const
{
	const UBmrSkeletalMeshComponent* MeshComponent = GetMeshComponent();
	return MeshComponent && MeshComponent->IsSkinAvailable(MeshComponent->GetAppliedSkinIndex());
}

// Returns the Skeletal Mesh of the Bomber character
UBmrSkeletalMeshComponent* UNMMSpotComponent::GetMeshComponent() const
{
	return GetOwner()->FindComponentByClass<UBmrSkeletalMeshComponent>();
}

UBmrSkeletalMeshComponent& UNMMSpotComponent::GetMeshChecked() const
{
	UBmrSkeletalMeshComponent* Mesh = GetMeshComponent();
	checkf(Mesh, TEXT("'Mesh' is nullptr, can not get mesh for '%s' spot."), *GetNameSafe(this));
	return *Mesh;
}

// Sets the look of this spot to the in-game player character
void UNMMSpotComponent::ApplyMeshOnPlayer()
{
	const ABmrPawn* Pawn = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	if (!ensureMsgf(Pawn, TEXT("ASSERT: [%i] %hs:\n'Pawn' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Update the chosen player mesh on the level
	const FBmrMeshData& PlayerMeshData = GetMeshChecked().GetMeshData();
	UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(Pawn);
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->SetReplicatedMeshData(PlayerMeshData);
}

/*********************************************************************************************
 * Cinematics
 ********************************************************************************************* */

// Returns main cinematic of this spot
ULevelSequence* UNMMSpotComponent::GetMasterSequence() const
{
	return MasterPlayer ? Cast<ULevelSequence>(MasterPlayer->GetSequence()) : nullptr;
}

// Prevents the spot from playing any cinematic
void UNMMSpotComponent::StopMasterSequence()
{
	if (MasterPlayer
	    && MasterPlayer->IsPlaying())
	{
		SetCinematicByState(ENMMState::None);
	}
}

// Returns true if current game state can be eventually changed
bool UNMMSpotComponent::CanChangeCinematicState(ENMMState NewMainMenuState) const
{
	if (CinematicState == NewMainMenuState)
	{
		return false;
	}

	const ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC || !MyPC->IsLocalController())
	{
		// Don't play cinematics for non-local players
		return false;
	}

	// Don't change any states if game is run from the Render Movie
	return !MyPC->bCinematicMode;
}

// Activate given cinematic state on this spot
void UNMMSpotComponent::SetCinematicByState(ENMMState MainMenuState)
{
	if (!CanChangeCinematicState(MainMenuState))
	{
		return;
	}

	if (MainMenuState == ENMMState::Transition)
	{
		// Don't set Transition state, instead apply idle while camera is moving
		MainMenuState = ENMMState::Idle;
	}

	const ENMMState PrevState = CinematicState;
	CinematicState = MainMenuState;

	if (PrevState != MainMenuState)
	{
		ApplyCinematicState();
	}
}

/*********************************************************************************************
 * Protected functions
 ********************************************************************************************* */

// Overridable native event for when play begins for this actor.
void UNMMSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	if (!World
	    || World->bIsTearingDown
	    || World->IsNetMode(NM_DedicatedServer))
	{
		// Don't process spot if world is restarting (which could happen since module could be loaded very late, right after request of restarting a level)
		// or if it's a dedicated server (when client-only mode is running)
		return;
	}

	// Skeletal mesh actor should own this tag, used to prevent initializing menu spots on other skeletal mesh actors, like from cinematics
	static const FName ExpectedTagName = NmmGameplayTags::Menu::Spot.GetTag().GetTagName();
	if (!GetOwner()->ActorHasTag(ExpectedTagName))
	{
		UE_LOG(LogBomber, Log, TEXT("[%i] %hs: Skip initializing '%s' spot for '%s' actor, it doesn't have '%s' tag."),
		    __LINE__, __FUNCTION__, *GetNameSafe(this), *GetNameSafe(GetOwner()), *ExpectedTagName.ToString());
		return;
	}

	UDalSubsystem::Get().ListenForDataAsset<UNMMDataAsset>(this, &ThisClass::OnDataAssetLoaded);
}

// Clears all transient data created by this component
void UNMMSpotComponent::OnUnregister()
{
	CinematicRow = FBmrCinematicRow::Empty;

	// Kill current cinematic player
	if (IsValid(MasterPlayer))
	{
		StopMasterSequence();
		MasterPlayer->ConditionalBeginDestroy();
		MasterPlayer = nullptr;
	}

	if (UNMMSpotsSubsystem* Subsystem = UNMMUtils::GetSpotsSubsystem(this))
	{
		Subsystem->RemoveMainMenuSpot(this);
	}

	Super::OnUnregister();
}

// Obtains and caches cinematic data from DR_Cinematics Data Registry to this spot
void UNMMSpotComponent::UpdateCinematicData()
{
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	const UDataRegistry* Registry = DRSubsystem ? DRSubsystem->GetRegistryForType(FDataRegistryType(FBmrCinematicRow::CinematicsRegistryTypeName)) : nullptr;
	if (!ensureMsgf(Registry, TEXT("ASSERT: [%i] %hs:\n'DR_Cinematics' Data Registry is not found, make sure it is created with '%s' type"), __LINE__, __FUNCTION__, *FBmrCinematicRow::CinematicsRegistryTypeName.ToString()))
	{
		return;
	}

	const FBmrPlayerTag& PlayerTag = GetMeshChecked().GetPlayerTag();

	int32 RowIndex = 0;
	Registry->ForEachCachedItem<FBmrCinematicRow>(TEXT("UpdateCinematicData"), [this, &PlayerTag, &RowIndex](const FName& /*Name*/, const FBmrCinematicRow& Row)
	{
		if (!CinematicRow.IsEmpty())
		{
			return;
		}

		if (Row.PlayerTag == PlayerTag)
		{
			CinematicRow = Row;
			CinematicRow.RowIndex = RowIndex;
		}
		++RowIndex;
	});
}

// Loads cinematic of this spot
void UNMMSpotComponent::LoadMasterSequencePlayer()
{
	if (MasterPlayer)
	{
		// Is already created
		return;
	}

	const TSoftObjectPtr<ULevelSequence> FoundMasterSequence = CinematicRow.LevelSequence;
	if (!ensureMsgf(!FoundMasterSequence.IsNull(), TEXT("'LevelSequenceToLoad' is not found, can not play cinematic for '%s' spot."), *GetNameSafe(this)))
	{
		return;
	}

	const TAsyncLoadPriority Priority = IsCurrentSpot() ? FStreamableManager::AsyncLoadHighPriority : FStreamableManager::DefaultAsyncLoadPriority;
	UAsyncLoadUtilsLibrary::AsyncLoadAsset(this, FoundMasterSequence, &ThisClass::OnMasterSequenceLoaded, Priority);
}

// Marks own cinematic as seen by player for the save system
void UNMMSpotComponent::MarkCinematicAsSeen()
{
	if (!IsCurrentSpot())
	{
		// Since there are multiple spots, only current one should mark cinematic as seen
		return;
	}

	if (UNMMSaveGameData* SaveGameData = UNMMUtils::GetSaveGameData())
	{
		SaveGameData->MarkCinematicAsSeen(CinematicRow.RowIndex);
	}
}

// Triggers or stops cinematic by current state
void UNMMSpotComponent::ApplyCinematicState()
{
	// --- Load cinematic synchronously if not loaded yet
	const bool bIsCinematicLoading = !MasterPlayer || CinematicRow.LevelSequence.IsPending();
	if (bIsCinematicLoading)
	{
		OnMasterSequenceLoaded(CinematicRow.LevelSequence.LoadSynchronous());
	}
	checkf(MasterPlayer, TEXT("ERROR: [%i] %s:\n'MasterPlayer' is null!"), __LINE__, *FString(__FUNCTION__));

	// --- Set the length of the cinematic
	constexpr int32 FirstFrame = 0;
	const int32 TotalFrames = UNMMUtils::GetCinematicTotalFrames(CinematicState, MasterPlayer);
	MasterPlayer->SetFrameRange(FirstFrame, TotalFrames);

	// --- Set the playback settings
	const FMovieSceneSequencePlaybackSettings& PlaybackSettings = UNMMUtils::GetCinematicSettings(CinematicState);
	MasterPlayer->SetPlaybackSettings(PlaybackSettings);

	// --- Set the playback position
	const FMovieSceneSequencePlaybackParams PlaybackPositionParams = UNMMUtils::GetPlaybackPositionParams(CinematicState, MasterPlayer);
	MasterPlayer->SetPlaybackPosition(PlaybackPositionParams);

	if (CinematicState == ENMMState::None)
	{
		// No need to stop it physically as playback settings above already paused a sequence
		return;
	}

	MasterPlayer->Play();

	if (CinematicState == ENMMState::Cinematic)
	{
		const ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
		checkf(MyPC, TEXT("ERROR: [%i] %hs:\n'MyPC' is null, local controller can not be obtained, cinematic can not be played!"), __LINE__, __FUNCTION__);
		MyPC->OnAnyCinematicStarted.Broadcast(CinematicRow.LevelSequence.Get(), this);
	}
}

// Is called when the cinematic was loaded to finish creation
void UNMMSpotComponent::OnMasterSequenceLoaded(ULevelSequence* LoadedMasterSequence)
{
	if (MasterPlayer)
	{
		// Is already initialized
		return;
	}

	// Create and cache the master sequence
	ALevelSequenceActor* OutActor = nullptr;
	MasterPlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(this, LoadedMasterSequence, {}, OutActor);
	checkf(MasterPlayer, TEXT("ERROR: 'MasterPlayer' was not created, something went wrong!"));

	// Override the aspect ratio of the cinematic to the aspect ratio of the screen
	FLevelSequenceCameraSettings CameraSettings;
	CameraSettings.bOverrideAspectRatioAxisConstraint = true;
	CameraSettings.AspectRatioAxisConstraint = UUtilsLibrary::GetViewportAspectRatioAxisConstraint();
	MasterPlayer->Initialize(GetMasterSequence(), GetWorld()->PersistentLevel, CameraSettings);

	if (IsCurrentSpot())
	{
		// This is active spot has created master sequence, start playing to let Engine preload tracks
		SetCinematicByState(ENMMState::Idle);

		// Notify that the active spot is ready and finished loading
		UNMMSpotsSubsystem::Get().OnActiveMenuSpotReady.Broadcast(this);
	}

	// Bind to react on cinematic finished, is pause instead of stop because of Settings.bPauseAtEnd
	MasterPlayer->OnPause.AddUniqueDynamic(this, &ThisClass::OnMasterSequencePaused);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the NMM data asset is loaded and available
void UNMMSpotComponent::OnDataAssetLoaded_Implementation(const UNMMDataAsset* DataAsset)
{
	UNMMSpotsSubsystem::Get().AddNewMainMenuSpot(this);

	UpdateCinematicData();

	if (!CinematicRow.IsEmpty())
	{
		LoadMasterSequencePlayer();
	}

	UNMMCameraSubsystem::Get().OnCameraRailTransitionStateChanged.AddUniqueDynamic(this, &ThisClass::OnCameraRailTransitionStateChanged);

	BIND_ON_MENU_STATE_CHANGED(this, ThisClass::OnNewMainMenuStateChanged);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);
}

// Called when the current game state was changed
void UNMMSpotComponent::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (!IsCurrentSpot())
	{
		// Don't handle inactive spot
		return;
	}

	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		// Reset the sequence to the beginning to make it ready for the next play
		constexpr bool bKeepCamera = true;
		UCinematicUtils::ResetSequence(MasterPlayer, bKeepCamera);
	}
}

// Called wen the Main Menu state was changed
void UNMMSpotComponent::OnNewMainMenuStateChanged_Implementation(ENMMState NewState, ENMMState PreviousState)
{
	if (NewState == ENMMState::BasicMenu)
	{
		return;
	}

	const bool bIsCurrentSpot = IsCurrentSpot();

	switch (NewState)
	{
		case ENMMState::Idle:
			if (bIsCurrentSpot)
			{
				ApplyMeshOnPlayer();
			}
			else
			{
				// Stop other spots from playing their cinematic
				StopMasterSequence();
			}
			break;
		case ENMMState::Cinematic:
			if (bIsCurrentSpot)
			{
				MarkCinematicAsSeen();
			}
			break;
		default: break;
	}

	if (bIsCurrentSpot)
	{
		SetCinematicByState(NewState);

		// Change the camera according to the cinematic state
		// Do it after the cinematic is played, otherwise camera will fail to obtain from not loaded sequence
		UNMMCameraSubsystem::Get().PossessCamera(NewState);
	}
}

// Called when the sequence is paused or when cinematic was ended
void UNMMSpotComponent::OnMasterSequencePaused_Implementation()
{
	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC
	    || UNMMUtils::GetMainMenuState() != ENMMState::Cinematic)
	{
		// Don't handle if not playing Main Part or is not local player
		return;
	}

	const FFrameNumber CurrentFrame = MasterPlayer->GetCurrentTime().Time.FrameNumber;
	const FFrameNumber EndFrame(UCinematicUtils::GetSequenceTotalFrames(GetMasterSequence()) - 1);
	if (CurrentFrame >= EndFrame)
	{
		// Notify that pre-game cinematic finished playing naturally
		FGameplayEventData EventData;
		EventData.EventTag = NmmGameplayTags::Event::CinematicPlaybackFinished;
		EventData.Instigator = MyPC->GetPawn();
		UGlobalMessageSubsystem::BroadcastGlobalMessage(EventData);
	}
}

// Called when the Camera Rail transition state changed
void UNMMSpotComponent::OnCameraRailTransitionStateChanged_Implementation(ENMMCameraRailTransitionState CameraRailTransitionStateChanged)
{
	switch (CameraRailTransitionStateChanged)
	{
		case ENMMCameraRailTransitionState::HalfwayTransition:
			if (IsCurrentSpot())
			{
				ApplyMeshOnPlayer();
			}
			break;
		default: break;
	}
}
