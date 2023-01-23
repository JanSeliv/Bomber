// Copyright (c) Yevhenii Selivanov

#include "Subsystems/SoundsManager.h"
//---
#include "GeneratedMap.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "DataAssets/SoundsDataAsset.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#include "Components/AudioComponent.h"

// Returns the Sounds Manager checked
USoundsManager& USoundsManager::Get()
{
	const UWorld* World = USingletonLibrary::Get().GetWorld();
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	const TSubclassOf<USoundsManager> SoundsManagerClass = USoundsDataAsset::Get().GetSoundsManagerClass();
	checkf(SoundsManagerClass, TEXT("%s: 'SoundsManagerClass' is null"), *FString(__FUNCTION__));
	USoundsManager* SoundsManager = Cast<USoundsManager>(World->GetSubsystemBase(SoundsManagerClass));
	checkf(SoundsManager, TEXT("%s: 'SoundsManager' is null"), *FString(__FUNCTION__));
	return *SoundsManager;
}

// Set new sound volume
void USoundsManager::SetSoundVolumeByClass(USoundClass* InSoundClass, float InVolume)
{
	USoundMix* MainSoundMix = USoundsDataAsset::Get().GetMainSoundMix();
	static constexpr float Pitch = 1.f;
	static constexpr float FadeInTime = 0.f;
	UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, InSoundClass, InVolume, Pitch, FadeInTime);
}

// Set the general sound volume for all sound classes in game
void USoundsManager::SetMasterVolume(double InVolume)
{
	MasterVolumeInternal = InVolume;

	USoundClass* MasterSoundClass = USoundsDataAsset::Get().GetMasterSoundClass();
	SetSoundVolumeByClass(MasterSoundClass, InVolume);
}

// Set new sound volume for music sound class
void USoundsManager::SetMusicVolume(double InVolume)
{
	MusicVolumeInternal = InVolume;

	USoundClass* MusicSoundClass = USoundsDataAsset::Get().GetMusicSoundClass();
	SetSoundVolumeByClass(MusicSoundClass, InVolume);
}

// Set new sound volume for SFX sound class
void USoundsManager::SetSFXVolume(double InVolume)
{
	SFXVolumeInternal = InVolume;

	USoundClass* SFXSoundClass = USoundsDataAsset::Get().GetSFXSoundClass();
	SetSoundVolumeByClass(SFXSoundClass, InVolume);
}

// Play the background music for current game state and level
void USoundsManager::PlayCurrentBackgroundMusic()
{
	const ECurrentGameState GameState = AMyGameStateBase::GetCurrentGameState();
	const ELevelType LevelType = USingletonLibrary::GetLevelType();
	USoundBase* BackgroundMusic = USoundsDataAsset::Get().GetBackgroundMusic(GameState, LevelType);
	if (!BackgroundMusic)
	{
		// Background music is not found for current state or level
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

// Play the blast sound of the bomb
void USoundsManager::PlayExplosionSFX()
{
	if (AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	if (USoundBase* ExplosionSFX = USoundsDataAsset::Get().GetExplosionSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ExplosionSFX);
	}
}

// Play the sound of the picked power-up
void USoundsManager::PlayItemPickUpSFX()
{
	if (AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	if (USoundBase* ItemPickUpSFX = USoundsDataAsset::Get().GetItemPickUpSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ItemPickUpSFX);
	}
}

/** Play the sound that is played right before the match ends. */
void USoundsManager::PlayEndGameCountdownSFX()
{
	if (AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
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
void USoundsManager::StopEndGameCountdownSFX()
{
	if (ActiveEndGameCountdownSFX && ActiveEndGameCountdownSFX->IsPlaying())
	{
		ActiveEndGameCountdownSFX->Stop();
	}
}

// Play the sound that is played before the match starts. 
void USoundsManager::PlayStartGameCountdownSFX()
{
	if (AMyGameStateBase::GetCurrentGameState() != ECGS::GameStarting)
	{
		return;
	}

	if (USoundBase* StarGameCountdownSFX = USoundsDataAsset::Get().GetStartGameCountdownSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), StarGameCountdownSFX);
	}
}

// Play the sound of the clicked UI element
void USoundsManager::PlayUIClickSFX()
{
	if (AMyGameStateBase::GetCurrentGameState() == ECGS::None)
	{
		return;
	}

	if (USoundBase* UIClickSFX = USoundsDataAsset::Get().GetUIClickSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), UIClickSFX);
	}
}

// Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors
void USoundsManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (IS_TRANSIENT(this))
	{
		return;
	}

	OnBeginPlay();

	USoundMix* MainSoundMix = USoundsDataAsset::Get().GetMainSoundMix();
	UGameplayStatics::SetBaseSoundMix(&InWorld, MainSoundMix);

	// Listed the ending the current game to play the End-Game sound on
	if (AMyPlayerState* CurrentPlayerState = USingletonLibrary::GetLocalPlayerState())
	{
		CurrentPlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
	}

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
		AGeneratedMap::Get().OnSetNewLevelType.AddUniqueDynamic(this, &ThisClass::OnGameLevelChanged);
	}

	PlayCurrentBackgroundMusic();
}

// Is called on ending the current game to play the End-Game sound
void USoundsManager::OnEndGameStateChanged(EEndGameState EndGameState)
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
void USoundsManager::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	StopEndGameCountdownSFX();
	PlayCurrentBackgroundMusic();
}

// Listen game levels to switch main menu background music
void USoundsManager::OnGameLevelChanged(ELevelType CurrentLevelType)
{
	PlayCurrentBackgroundMusic();
}
