// Copyright (c) Yevhenii Selivanov

#include "Subsystems/SoundsSubsystem.h"
//---
#include "Bomber.h"
#include "DataAssets/SoundsDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SoundsSubsystem)

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
bool USoundsSubsystem::CanPlaySounds() const
{
	const UWorld* World = GetWorld();
	return World && World->bAllowAudioPlayback && !World->IsNetMode(NM_DedicatedServer);
}

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

// Play the background music for current game state and level
void USoundsSubsystem::PlayCurrentBackgroundMusic()
{
	if (!CanPlaySounds())
	{
		return;
	}

	const ECurrentGameState GameState = AMyGameStateBase::GetCurrentGameState();
	const ELevelType LevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	USoundBase* BackgroundMusic = USoundsDataAsset::Get().GetBackgroundMusic(GameState, LevelType);

	if (!BackgroundMusic)
	{
		// Background music is not found for current state or level, disable current
		StopCurrentBackgroundMusic();
		return;
	}

	if (!BackgroundMusicComponentInternal)
	{
		BackgroundMusicComponentInternal = UGameplayStatics::CreateSound2D(GetWorld(), BackgroundMusic);
		check(BackgroundMusicComponentInternal);
	}

	if (BackgroundMusicComponentInternal->IsPlaying()
	    && BackgroundMusicComponentInternal->GetSound() == BackgroundMusic)
	{
		// Do not switch music since is the same
		return;
	}

	BackgroundMusicComponentInternal->SetSound(BackgroundMusic);
	BackgroundMusicComponentInternal->Play();
}

// Stops currently played background music
void USoundsSubsystem::StopCurrentBackgroundMusic()
{
	if (BackgroundMusicComponentInternal
	    && BackgroundMusicComponentInternal->IsPlaying())
	{
		BackgroundMusicComponentInternal->Stop();
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

	if (USoundBase* EndGameCountdownTimerSFX = USoundsDataAsset::Get().GetEndGameCountdownSFX())
	{
		ActiveEndGameCountdownSFX = UGameplayStatics::CreateSound2D(GetWorld(), EndGameCountdownTimerSFX);
		ActiveEndGameCountdownSFX->Play();
	}
}

// Stops the sound that is played right before the match ends.
void USoundsSubsystem::StopEndGameCountdownSFX()
{
	if (ActiveEndGameCountdownSFX && ActiveEndGameCountdownSFX->IsPlaying())
	{
		ActiveEndGameCountdownSFX->Stop();
	}
}

// Play the sound that is played before the match starts. 
void USoundsSubsystem::PlayStartGameCountdownSFX()
{
	if (!CanPlaySounds()
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::GameStarting)
	{
		return;
	}

	if (USoundBase* StarGameCountdownSFX = USoundsDataAsset::Get().GetStartGameCountdownSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), StarGameCountdownSFX);
	}
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

	// Listed the ending the current game to play the End-Game sound on
	UGlobalEventsSubsystem::Get().OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);

	BIND_AND_CALL_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	PlayCurrentBackgroundMusic();
}

// Is called on ending the current game to play the End-Game sound
void USoundsSubsystem::OnEndGameStateChanged(EEndGameState EndGameState)
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
void USoundsSubsystem::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	StopEndGameCountdownSFX();
	PlayCurrentBackgroundMusic();
}

