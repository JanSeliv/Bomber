// Copyright (c) Yevhenii Selivanov

#include "SoundsManager.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/SingletonLibrary.h"

const USoundsDataAsset& USoundsDataAsset::Get()
{
	const USoundsDataAsset* SoundsDataAsset = USingletonLibrary::GetSoundsDataAsset();
	checkf(SoundsDataAsset, TEXT("The Sounds Data Asset is not valid"));
	return *SoundsDataAsset;
}

// Returns the sound manager
USoundsManager* USoundsDataAsset::GetSoundsManager() const
{
	if (!SoundManager)
	{
		UWorld* World = USingletonLibrary::Get().GetWorld();
		SoundManager = NewObject<USoundsManager>(World, SoundsManagerClass, NAME_None, RF_Public | RF_Transactional);
	}

	return SoundManager;
}

// Returns the End-Game sound by specified End-Game state
USoundBase* USoundsDataAsset::GetEndGameSFX(EEndGameState EndGameState) const
{
	if (const TObjectPtr<USoundBase>* FoundSFX = EndGameSFXInternal.Find(EndGameState))
	{
		return *FoundSFX;
	}

	return nullptr;
}

// Returns a world of stored level map
UWorld* USoundsManager::GetWorld() const
{
	return USingletonLibrary::Get().GetWorld();
}

// Set new sound volume
void USoundsManager::SetSoundVolumeByClass(USoundClass* InSoundClass, float InVolume)
{
	const UWorld* World = USingletonLibrary::Get().GetWorld();
	USoundMix* MainSoundMix = USoundsDataAsset::Get().GetMainSoundMix();
	static constexpr float Pitch = 1.f;
	static constexpr float FadeInTime = 0.f;
	UGameplayStatics::SetSoundMixClassOverride(World, MainSoundMix, InSoundClass, InVolume, Pitch, FadeInTime);
}

// Set the general sound volume for all sound classes in game
void USoundsManager::SetMasterVolume(float InVolume)
{
	MasterVolumeInternal = InVolume;

	USoundClass* MasterSoundClass = USoundsDataAsset::Get().GetMasterSoundClass();
	SetSoundVolumeByClass(MasterSoundClass, InVolume);
}

// Set new sound volume for music sound class
void USoundsManager::SetMusicVolume(float InVolume)
{
	MusicVolumeInternal = InVolume;

	USoundClass* MusicSoundClass = USoundsDataAsset::Get().GetMusicSoundClass();
	SetSoundVolumeByClass(MusicSoundClass, InVolume);
}

// Set new sound volume for SFX sound class
void USoundsManager::SetSFXVolume(float InVolume)
{
	SFXVolumeInternal = InVolume;

	USoundClass* SFXSoundClass = USoundsDataAsset::Get().GetSFXSoundClass();
	SetSoundVolumeByClass(SFXSoundClass, InVolume);
}

// Play the blast sound of the bomb
void USoundsManager::PlayExplosionSFX()
{
	if (AMyGameStateBase::GetCurrentGameState(this) != ECGS::InGame)
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
	if (AMyGameStateBase::GetCurrentGameState(this) != ECGS::InGame)
	{
		return;
	}

	if (USoundBase* ItemPickUpSFX = USoundsDataAsset::Get().GetItemPickUpSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ItemPickUpSFX);
	}
}

// Play the sound of the clicked UI element
void USoundsManager::PlayUIClickSFX()
{
	if (USoundBase* UIClickSFX = USoundsDataAsset::Get().GetUIClickSFXInternal())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), UIClickSFX);
	}
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config
void USoundsManager::PostInitProperties()
{
	UObject::PostInitProperties();

	if (IS_TRANSIENT(this))
	{
		return;
	}

	if (USingletonLibrary::HasWorldBegunPlay())
	{
		BeginPlay();
		OnBeginPlay();
	}
}

// Called when the game starts
void USoundsManager::BeginPlay()
{
	const UWorld* World = GetWorld();

	USoundMix* MainSoundMix = USoundsDataAsset::Get().GetMainSoundMix();
	UGameplayStatics::SetBaseSoundMix(World, MainSoundMix);

	USoundBase* BackgroundSound = USoundsDataAsset::Get().GetBackgroundSound();
	UGameplayStatics::SpawnSound2D(World, BackgroundSound);

	// Listed the ending the current game to play the End-Game sound on
	if (AMyPlayerState* CurrentPlayerState = USingletonLibrary::GetLocalPlayerState())
	{
		CurrentPlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
	}
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
