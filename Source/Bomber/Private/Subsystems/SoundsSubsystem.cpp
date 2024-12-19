// Copyright (c) Yevhenii Selivanov

#include "Subsystems/SoundsSubsystem.h"
//---
#include "Bomber.h"
#include "DataAssets/SoundsDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SoundsSubsystem)

/*********************************************************************************************
 * Static methods
 ********************************************************************************************* */

// Returns the Sounds Manager, is checked and wil crash if can't be obtained
USoundsSubsystem& USoundsSubsystem::Get()
{
	USoundsSubsystem* SoundsSubsystem = GetSoundsSubsystem();
	checkf(SoundsSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *SoundsSubsystem;
}

// Returns the pointer to the Sounds Manager
USoundsSubsystem* USoundsSubsystem::GetSoundsSubsystem(const UObject* WorldContextObject)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	const TSubclassOf<USoundsSubsystem> SoundsSubsystemClass = USoundsDataAsset::Get().GetSoundsSubsystemClass();
	return World ? Cast<USoundsSubsystem>(World->GetSubsystemBase(SoundsSubsystemClass)) : nullptr;
}

// Returns true if sounds can be played
bool USoundsSubsystem::CanPlaySounds()
{
	if (!GEngine || !GEngine->UseSound())
	{
		return false;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	return World
	       && World->bAllowAudioPlayback
	       && !World->IsNetMode(NM_DedicatedServer);
}

// Play the sound in 2D space with ensuring that this sound component is created only once
void USoundsSubsystem::PlaySingleSound2D(USoundBase* InSound)
{
	if (!CanPlaySounds()
	    || !ensureMsgf(InSound, TEXT("ASSERT: [%i] %hs:\n'InSound' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const TObjectPtr<UAudioComponent>* SoundComponentPtr = SoundComponentsInternal.Find(InSound);
	if (!SoundComponentPtr)
	{
		UAudioComponent* NewSoundComponent = UGameplayStatics::SpawnSound2D(GetWorld(), InSound);
		checkf(NewSoundComponent, TEXT("ERROR: [%i] %hs:\n'NewSoundComponent' failed to create from '%s' sound!"), __LINE__, __FUNCTION__, *GetNameSafe(InSound));

		// Remember the sound component to reuse all next plays
		SoundComponentPtr = &SoundComponentsInternal.Add(InSound, NewSoundComponent);
		checkf(SoundComponentPtr, TEXT("ERROR: [%i] %hs:\n'SoundComponentPtr' is null, failed to add '%s'!"), __LINE__, __FUNCTION__, *GetNameSafe(NewSoundComponent));

		// Disable auto destroy, so we can reuse it multiple times, otherwise sounds will be playing multiple times
		NewSoundComponent->bAutoDestroy = false;
	}

	UAudioComponent& SoundComponentRef = **SoundComponentPtr; // It's safe to dereference since both pointers are checked above

	if (SoundComponentRef.IsPlaying())
	{
		// Stop existing sound and play new one
		SoundComponentRef.Stop();
	}

	SoundComponentRef.Play();
}

// Deactivates the given sound if currently playing
void USoundsSubsystem::StopSingleSound2D(USoundBase* InSound)
{
	if (!ensureMsgf(InSound, TEXT("ASSERT: [%i] %hs:\n'InSound' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const TObjectPtr<UAudioComponent>* SoundComponentPtr = SoundComponentsInternal.Find(InSound);
	UAudioComponent* SoundComponent = SoundComponentPtr ? *SoundComponentPtr : nullptr;
	if (SoundComponent)
	{
		SoundComponent->Stop();
	}
}

// Destroy sound component by given sound, it's used to perform cleanup when game is finished
void USoundsSubsystem::DestroySingleSound2D(USoundBase* InSound)
{
	const TObjectPtr<UAudioComponent>* SoundComponentPtr = InSound ? SoundComponentsInternal.Find(InSound) : nullptr;
	if (!SoundComponentPtr)
	{
		// Is already unloaded
		return;
	}

	if (UAudioComponent* SoundComponent = *SoundComponentPtr)
	{
		SoundComponent->Stop();
		SoundComponent->DestroyComponent();
	}

	SoundComponentsInternal.Remove(InSound);
}

// Performs cleanup on all known sound components
void USoundsSubsystem::DestroyAllSoundComponents()
{
	for (TTuple<TObjectPtr<USoundBase>, TObjectPtr<UAudioComponent>>& SoundComponentPair : SoundComponentsInternal)
	{
		if (UAudioComponent* SoundComponent = SoundComponentPair.Value)
		{
			SoundComponent->Stop();
			SoundComponent->DestroyComponent();
		}
	}
	SoundComponentsInternal.Empty();
}

/*********************************************************************************************
 * Volume
 ********************************************************************************************* */

// Set new sound volume
void USoundsSubsystem::SetSoundVolumeByClass(USoundClass* InSoundClass, float InVolume)
{
	if (!CanPlaySounds())
	{
		return;
	}

	USoundMix* MainSoundMix = USoundsDataAsset::Get().GetMainSoundMix();
	static constexpr float Pitch = 1.f;
	static constexpr float FadeInTime = 0.f;
	UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, InSoundClass, InVolume, Pitch, FadeInTime);
}

// Set the general sound volume for all sound classes in game
void USoundsSubsystem::SetMasterVolume(double InVolume)
{
	MasterVolumeInternal = InVolume;

	USoundClass* MasterSoundClass = USoundsDataAsset::Get().GetMasterSoundClass();
	SetSoundVolumeByClass(MasterSoundClass, InVolume);
}

// Set new sound volume for music sound class
void USoundsSubsystem::SetMusicVolume(double InVolume)
{
	MusicVolumeInternal = InVolume;

	USoundClass* MusicSoundClass = USoundsDataAsset::Get().GetMusicSoundClass();
	SetSoundVolumeByClass(MusicSoundClass, InVolume);
}

// Set new sound volume for SFX sound class
void USoundsSubsystem::SetSFXVolume(double InVolume)
{
	SFXVolumeInternal = InVolume;

	USoundClass* SFXSoundClass = USoundsDataAsset::Get().GetSFXSoundClass();
	SetSoundVolumeByClass(SFXSoundClass, InVolume);
}

// Trigger the background music to be played during the match
void USoundsSubsystem::PlayInGameMusic()
{
	if (!CanPlaySounds())
	{
		return;
	}

	const ELevelType LevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	USoundBase* InGameMusic = USoundsDataAsset::Get().GetInGameMusic(LevelType);

	if (!InGameMusic)
	{
		// Background music is not found for current state or level, disable current
		StopInGameMusic();
		return;
	}

	PlaySingleSound2D(InGameMusic);
}

// Stops currently played in-match background music
void USoundsSubsystem::StopInGameMusic()
{
	const ELevelType LevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	if (USoundBase* InGameMusic = USoundsDataAsset::Get().GetInGameMusic(LevelType))
	{
		StopSingleSound2D(InGameMusic);
	}
}

// Play the blast sound of the bomb
void USoundsSubsystem::PlayExplosionSFX()
{
	if (!CanPlaySounds()
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	if (USoundBase* ExplosionSFX = USoundsDataAsset::Get().GetExplosionSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ExplosionSFX);
	}
}

// Play the sound of the picked power-up
void USoundsSubsystem::PlayItemPickUpSFX()
{
	if (!CanPlaySounds()
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	if (USoundBase* ItemPickUpSFX = USoundsDataAsset::Get().GetItemPickUpSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ItemPickUpSFX);
	}
}

/** Play the sound that is played right before the match ends. */
void USoundsSubsystem::PlayEndGameCountdownSFX()
{
	if (!CanPlaySounds()
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	PlaySingleSound2D(USoundsDataAsset::Get().GetEndGameCountdownSFX());
}

// Stops the sound that is played right before the match ends.
void USoundsSubsystem::StopEndGameCountdownSFX()
{
	StopSingleSound2D(USoundsDataAsset::Get().GetEndGameCountdownSFX());
}

// Play the sound that is played before the match starts. 
void USoundsSubsystem::PlayStartGameCountdownSFX()
{
	if (!CanPlaySounds()
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::GameStarting)
	{
		return;
	}

	PlaySingleSound2D(USoundsDataAsset::Get().GetStartGameCountdownSFX());
}

void USoundsSubsystem::StopStartGameCountdownSFX()
{
	StopSingleSound2D(USoundsDataAsset::Get().GetStartGameCountdownSFX());
}

// Play the sound of the clicked UI element
void USoundsSubsystem::PlayUIClickSFX()
{
	if (!CanPlaySounds()
	    || AMyGameStateBase::GetCurrentGameState() == ECGS::None)
	{
		return;
	}

	if (USoundBase* UIClickSFX = USoundsDataAsset::Get().GetUIClickSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), UIClickSFX);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors
void USoundsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (IS_TRANSIENT(this)
	    || !CanPlaySounds())
	{
		return;
	}

	OnBeginPlay();

	USoundMix* MainSoundMix = USoundsDataAsset::Get().GetMainSoundMix();
	UGameplayStatics::SetBaseSoundMix(&InWorld, MainSoundMix);

	BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Is overridden to perform cleanup on ending the game
void USoundsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DestroyAllSoundComponents();
}

// Is called on ending the current game to play the End-Game sound
void USoundsSubsystem::OnEndGameStateChanged_Implementation(EEndGameState EndGameState)
{
	if (EndGameState == EEndGameState::None)
	{
		return;
	}

	if (USoundBase* EndGameSFX = USoundsDataAsset::Get().GetEndGameSFX(EndGameState))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), EndGameSFX);
	}
}

// Listen game states to switch background music
void USoundsSubsystem::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECGS::GameStarting:
			PlayInGameMusic();
			PlayStartGameCountdownSFX();
			break;
		case ECGS::Menu:
			StopInGameMusic();
			StopStartGameCountdownSFX();
			break;
		case ECGS::EndGame:
			StopEndGameCountdownSFX();
			break;
		default:
			break;
	}
}

// Called when the local player state is initialized and its assigned character is ready
void USoundsSubsystem::OnLocalPlayerStateReady_Implementation(class AMyPlayerState* PlayerState, int32 CharacterID)
{
	// Listen the ending the current game to play the End-Game sound on
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}