// Copyright (c) Yevhenii Selivanov

#include "Components/NMMSpotComponent.h"

// NMM
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Controllers/BmrPlayerController.h"
#include "DalSubsystem.h"
#include "DataRegistries/BmrPlayerRow.h"
#include "DataRegistries/BmrPlayerSkinRow.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
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

// Returns true if this spot is visible, unlocked, has loaded cinematic and can be selected by player
bool UNMMSpotComponent::IsSpotAvailable() const
{
	const UBmrSkeletalMeshComponent* MeshComponent = GetMeshComponent();
	return IsActive()
	       && MasterPlayer
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

// Returns the owner of this component as Bomber Skeletal Mesh actor
ABmrSkeletalMeshActor& UNMMSpotComponent::GetOwnerChecked() const
{
	return *CastChecked<ABmrSkeletalMeshActor>(GetOwner());
}

// Sets the look of this spot to the in-game player character
void UNMMSpotComponent::ApplyMeshOnPlayer()
{
	ABmrPlayerState* PlayerState = UBmrBlueprintFunctionLibrary::GetLocalPlayerState();
	if (!ensureMsgf(PlayerState, TEXT("ASSERT: [%i] %hs:\n'PlayerState' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Spot might have None skin by default, apply first skin then
	FBmrMeshData PlayerMeshData = GetMeshChecked().GetMeshData();
	const FBmrPlayerRow* FallbackPlayerRow = PlayerMeshData.SkinRowName.IsNone() ? FBmrPlayerRow::GetRowByName(PlayerMeshData.RowName) : nullptr;
	PlayerMeshData.SkinRowName = FallbackPlayerRow ? FBmrPlayerSkinRow::GetSkinRowName(FallbackPlayerRow->PlayerTag, /*SkinIndex*/ 0) : PlayerMeshData.SkinRowName;

	// Persist the chosen player visual on the player state, replicates and survives pawn dies
	PlayerState->SetChosenMeshData(PlayerMeshData);
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
		SetCinematicByState(FNmmStateTag::None);
	}
}

// Returns true if current game state can be eventually changed
bool UNMMSpotComponent::CanChangeCinematicState(FNmmStateTag NewMenuState) const
{
	if (CinematicState == NewMenuState)
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
void UNMMSpotComponent::SetCinematicByState(FNmmStateTag MenuState)
{
	if (!CanChangeCinematicState(MenuState))
	{
		return;
	}

	if (MenuState == FNmmStateTag::Transition)
	{
		// Don't set Transition state, instead apply idle while camera is moving
		MenuState = FNmmStateTag::Idle;
	}

	const FNmmStateTag PrevState = CinematicState;
	CinematicState = MenuState;

	if (PrevState != MenuState)
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
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	// Clear cached CinematicPlaybackFinished so late-binding listeners receive fresh data on next menu load
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::CinematicPlaybackFinished, this);

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

// Reinitializes cinematic data from Data Registry: cleans up current cinematic and re-queries for a matching row
void UNMMSpotComponent::ReinitializeCinematicData()
{
	CinematicState = FNmmStateTag::None;
	CinematicRow = FBmrCinematicRow::Empty;

	if (IsValid(MasterPlayer))
	{
		// Discard pre-animated state, otherwise Stop() would trigger RestorePreAnimatedState() restoring wrong camera
		MasterPlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::ForceKeepState);

		MasterPlayer->Stop();
		MasterPlayer->ConditionalBeginDestroy();
		MasterPlayer = nullptr;
	}

	UpdateCinematicData();
}

// Obtains and caches cinematic data from DR_Cinematics Data Registry to this spot
void UNMMSpotComponent::UpdateCinematicData()
{
	const FBmrPlayerTag& PlayerTag = GetOwnerChecked().GetPlayerTag();
	if (!ensureMsgf(PlayerTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'PlayerTag' is not set on '%s' spot actor!"), __LINE__, __FUNCTION__, *GetNameSafe(GetOwner())))
	{
		return;
	}

	const FBmrCinematicRow* FoundRow = FBmrCinematicRow::GetRowByPredicate([&PlayerTag](const FBmrCinematicRow& Row)
	{
		return Row.PlayerTag == PlayerTag;
	});

	if (FoundRow)
	{
		CinematicRow = *FoundRow;
	}
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
		SaveGameData->MarkCinematicAsSeen(CinematicRow.Priority);
	}
}

// Triggers or stops cinematic by current state
void UNMMSpotComponent::ApplyCinematicState()
{
	if (!MasterPlayer)
	{
		// Async load is still in progress, will be applied by InitMasterSequencePlayer after batch load completes
		return;
	}

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

	if (CinematicState == FNmmStateTag::None)
	{
		// No need to stop it physically as playback settings above already paused a sequence
		return;
	}

	MasterPlayer->Play();

	// Force set first evaluation so camera-cut bindings resolve in this frame
	const FMovieSceneSequencePlaybackParams JumpParams(MasterPlayer->GetCurrentTime().Time, EUpdatePositionMethod::Jump);
	MasterPlayer->SetPlaybackPosition(JumpParams);

	if (CinematicState == FNmmStateTag::Cinematic)
	{
		const ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
		checkf(MyPC, TEXT("ERROR: [%i] %hs:\n'MyPC' is null, local controller can not be obtained, cinematic can not be played!"), __LINE__, __FUNCTION__);
		MyPC->OnAnyCinematicStarted.Broadcast(CinematicRow.LevelSequence.Get(), this);
	}
}

// Creates MasterPlayer from a loaded cinematic asset and notifies the Spots Subsystem, no-op if not yet loaded or already initialized
void UNMMSpotComponent::InitMasterSequencePlayer()
{
	if (MasterPlayer
	    || CinematicRow.IsEmpty())
	{
		// Is already initialized
		return;
	}

	// Cinematic assets are batch-loaded by the Spots Subsystem, resolve the soft pointer
	ULevelSequence* LoadedMasterSequence = CinematicRow.LevelSequence.Get();
	if (!ensureMsgf(LoadedMasterSequence, TEXT("ASSERT: [%i] %hs:\n'LoadedSequence' is not loaded for '%s' spot, should be called after batch load!"), __LINE__, __FUNCTION__, *GetNameSafe(this))
	    || CinematicRow.LevelSequence.Get() != LoadedMasterSequence)
	{
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

	// Bind to react on cinematic finished, is pause instead of stop because of Settings.bPauseAtEnd
	MasterPlayer->OnPause.AddUniqueDynamic(this, &ThisClass::OnMasterSequencePaused);

	// Notify subsystem that this spot finished loading, it will evaluate active spot once all spots are ready
	UNMMSpotsSubsystem::Get().NotifySpotLoaded(this);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the NMM data asset is loaded and available
void UNMMSpotComponent::OnDataAssetLoaded_Implementation(const UNMMDataAsset* DataAsset)
{
	UpdateCinematicData();

	UNMMSpotsSubsystem::Get().AddNewMainMenuSpot(this);

	UNMMCameraSubsystem::Get().OnCameraRailTransitionStateChanged.AddUniqueDynamic(this, &ThisClass::OnCameraRailTransitionStateChanged);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(NmmGameplayTags::Event::MenuStateChanged, this, &ThisClass::OnNewMainMenuStateChanged);

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

// Called when the Main Menu state was changed
void UNMMSpotComponent::OnNewMainMenuStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const FNmmStateTag NewState(Payload.InstigatorTags.First());
	if (NewState == FNmmStateTag::BasicMenu)
	{
		return;
	}

	const bool bIsCurrentSpot = IsCurrentSpot();

	if (NewState == FNmmStateTag::Idle)
	{
		if (bIsCurrentSpot)
		{
			ApplyMeshOnPlayer();
		}
		else
		{
			// Stop other spots from playing their cinematic
			StopMasterSequence();
		}
	}
	else if (NewState == FNmmStateTag::Cinematic
	         && bIsCurrentSpot)
	{
		MarkCinematicAsSeen();
	}

	if (bIsCurrentSpot)
	{
		SetCinematicByState(NewState);
	}
}

// Called when the sequence is paused or when cinematic was ended
void UNMMSpotComponent::OnMasterSequencePaused_Implementation()
{
	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC
	    || UNMMUtils::GetMainMenuState() != FNmmStateTag::Cinematic)
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
