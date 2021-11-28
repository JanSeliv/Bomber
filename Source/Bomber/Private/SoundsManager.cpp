// Copyright 2021 Yevhenii Selivanov

#include "SoundsManager.h"
//---
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

// Set new sound volume for music sound class
void USoundsManager::SetMusicVolume(float InVolume)
{
	USoundClass* MusicSoundClass = USoundsDataAsset::Get().GetMusicSoundClass();
	MusicVolumeInternal = InVolume;
	SetSoundVolumeByClass(MusicSoundClass, InVolume);
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
}
