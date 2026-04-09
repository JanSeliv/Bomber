// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrSoundsSubsystem.h"

// Bomber
#include "Bomber.h"
#include "DalRegistrySubsystem.h"
#include "DataAssets/BmrSoundsDataAsset.h"
#include "DataRegistries/BmrSoundsBackgroundRow.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrSoundsSubsystem)

/*********************************************************************************************
 * Static methods
 ********************************************************************************************* */

// Returns the Sounds Manager, is checked and wil crash if can't be obtained
UBmrSoundsSubsystem& UBmrSoundsSubsystem::Get()
{
	UBmrSoundsSubsystem* SoundsSubsystem = GetSoundsSubsystem();
	checkf(SoundsSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *SoundsSubsystem;
}

// Returns the pointer to the Sounds Manager
UBmrSoundsSubsystem* UBmrSoundsSubsystem::GetSoundsSubsystem(const UObject* WorldContextObject)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	const TSubclassOf<UBmrSoundsSubsystem> SoundsSubsystemClass = UBmrSoundsDataAsset::Get().GetSoundsSubsystemClass();
	return World ? Cast<UBmrSoundsSubsystem>(World->GetSubsystemBase(SoundsSubsystemClass)) : nullptr;
}

// Returns true if sounds can be played
bool UBmrSoundsSubsystem::CanPlaySounds()
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
void UBmrSoundsSubsystem::PlaySingleSound2D(USoundBase* InSound)
{
	if (!CanPlaySounds()
	    || !ensureMsgf(InSound, TEXT("ASSERT: [%i] %hs:\n'InSound' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const TObjectPtr<UAudioComponent>* SoundComponentPtr = SoundComponents.Find(InSound);
	if (!SoundComponentPtr)
	{
		UAudioComponent* NewSoundComponent = UGameplayStatics::SpawnSound2D(GetWorld(), InSound);
		checkf(NewSoundComponent, TEXT("ERROR: [%i] %hs:\n'NewSoundComponent' failed to create from '%s' sound!"), __LINE__, __FUNCTION__, *GetNameSafe(InSound));

		// Remember the sound component to reuse all next plays
		SoundComponentPtr = &SoundComponents.Add(InSound, NewSoundComponent);
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
void UBmrSoundsSubsystem::StopSingleSound2D(USoundBase* InSound)
{
	if (!ensureMsgf(InSound, TEXT("ASSERT: [%i] %hs:\n'InSound' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const TObjectPtr<UAudioComponent>* SoundComponentPtr = SoundComponents.Find(InSound);
	UAudioComponent* SoundComponent = SoundComponentPtr ? *SoundComponentPtr : nullptr;
	if (SoundComponent)
	{
		SoundComponent->Stop();
	}
}

// Destroy sound component by given sound, it's used to perform cleanup when game is finished
void UBmrSoundsSubsystem::DestroySingleSound2D(USoundBase* InSound)
{
	const TObjectPtr<UAudioComponent>* SoundComponentPtr = InSound ? SoundComponents.Find(InSound) : nullptr;
	UAudioComponent* SoundComponent = SoundComponentPtr ? *SoundComponentPtr : nullptr;
	if (!SoundComponent)
	{
		// Is already unloaded
		return;
	}

	SoundComponent->Stop();
	SoundComponent->DestroyComponent();

	// Release the sound and all its referenced assets
	constexpr bool bUnloadReferences = true;
	UModularGameFeaturePluginUtils::UnloadAsset(InSound, bUnloadReferences);

	SoundComponents.Remove(InSound);
}

// Performs cleanup on all known sound components
void UBmrSoundsSubsystem::DestroyAllSoundComponents()
{
	for (TTuple<TObjectPtr<USoundBase>, TObjectPtr<UAudioComponent>>& SoundComponentPair : SoundComponents)
	{
		if (UAudioComponent* SoundComponent = SoundComponentPair.Value)
		{
			SoundComponent->Stop();
			SoundComponent->DestroyComponent();
		}
	}
	SoundComponents.Empty();

#if WITH_EDITOR
	// Clean up all potentially leaked editor sounds (such as UScrubbedSound), firstly leaked in UE5.6.0
	const UWorld* World = GetWorld();
	const ULevel* Level = World ? World->GetCurrentLevel() : nullptr;
	if (Level)
	{
		TArray<UObject*> FoundObjects;
		GetObjectsWithOuter(Level, FoundObjects, false, RF_NoFlags, EInternalObjectFlags::None);
		for (UObject* ObjectIt : FoundObjects)
		{
			if (IsValid(ObjectIt)
			    && ObjectIt->IsA<USoundBase>()
			    && ObjectIt->HasAnyFlags(RF_Transient))
			{
				UModularGameFeaturePluginUtils::UnloadAsset(ObjectIt);
			}
		}
	}
#endif
}

/*********************************************************************************************
 * Volume
 ********************************************************************************************* */

// Set new sound volume
void UBmrSoundsSubsystem::SetSoundVolumeByClass(USoundClass* InSoundClass, float InVolume)
{
	if (!CanPlaySounds())
	{
		return;
	}

	USoundMix* MainSoundMix = UBmrSoundsDataAsset::Get().GetMainSoundMix();
	static constexpr float Pitch = 1.f;
	static constexpr float FadeInTime = 0.f;
	UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, InSoundClass, InVolume, Pitch, FadeInTime);
}

// Set the general sound volume for all sound classes in game
void UBmrSoundsSubsystem::SetMasterVolume(double InVolume)
{
	MasterVolume = InVolume;

	USoundClass* MasterSoundClass = UBmrSoundsDataAsset::Get().GetMasterSoundClass();
	SetSoundVolumeByClass(MasterSoundClass, InVolume);
}

// Set new sound volume for music sound class
void UBmrSoundsSubsystem::SetMusicVolume(double InVolume)
{
	MusicVolume = InVolume;

	USoundClass* MusicSoundClass = UBmrSoundsDataAsset::Get().GetMusicSoundClass();
	SetSoundVolumeByClass(MusicSoundClass, InVolume);
}

// Set new sound volume for SFX sound class
void UBmrSoundsSubsystem::SetSFXVolume(double InVolume)
{
	SFXVolume = InVolume;

	USoundClass* SFXSoundClass = UBmrSoundsDataAsset::Get().GetSFXSoundClass();
	SetSoundVolumeByClass(SFXSoundClass, InVolume);
}

// Trigger the background music to be played during the match
void UBmrSoundsSubsystem::PlayInGameMusic()
{
	if (!CanPlaySounds())
	{
		return;
	}

	const EBmrLevelType LevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	USoundBase* InGameMusic = UBmrSoundsDataAsset::Get().GetInGameMusic(LevelType);

	if (!InGameMusic)
	{
		// Background music is not found for current state or level, disable current
		StopInGameMusic();
		return;
	}

	PlaySingleSound2D(InGameMusic);
}

// Stops currently played in-match background music
void UBmrSoundsSubsystem::StopInGameMusic()
{
	const EBmrLevelType LevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	if (USoundBase* InGameMusic = UBmrSoundsDataAsset::Get().GetInGameMusic(LevelType))
	{
		StopSingleSound2D(InGameMusic);
	}
}

/** Play the sound that is played right before the match ends. */
void UBmrSoundsSubsystem::PlayEndGameCountdownSFX()
{
	if (!CanPlaySounds())
	{
		return;
	}

	const ABmrGameState* GameState = UBmrBlueprintFunctionLibrary::GetGameState();
	if (!GameState || !GameState->HasMatchingGameplayTag(FBmrGameStateTag::InGame))
	{
		return;
	}

	PlaySingleSound2D(UBmrSoundsDataAsset::Get().GetEndGameCountdownSFX());
}

// Stops the sound that is played right before the match ends.
void UBmrSoundsSubsystem::StopEndGameCountdownSFX()
{
	StopSingleSound2D(UBmrSoundsDataAsset::Get().GetEndGameCountdownSFX());
}

// Play the sound that is played before the match starts
void UBmrSoundsSubsystem::PlayStartGameCountdownSFX()
{
	if (!CanPlaySounds())
	{
		return;
	}

	const ABmrGameState* GameState = UBmrBlueprintFunctionLibrary::GetGameState();
	if (!GameState || !GameState->HasMatchingGameplayTag(FBmrGameStateTag::GameStarting))
	{
		return;
	}

	PlaySingleSound2D(UBmrSoundsDataAsset::Get().GetStartGameCountdownSFX());
}

void UBmrSoundsSubsystem::StopStartGameCountdownSFX()
{
	StopSingleSound2D(UBmrSoundsDataAsset::Get().GetStartGameCountdownSFX());
}

// Play the sound of the clicked UI element
void UBmrSoundsSubsystem::PlayUIClickSFX()
{
	if (!CanPlaySounds())
	{
		return;
	}

	if (!UBmrBlueprintFunctionLibrary::IsLocalPawnReady())
	{
		return;
	}

	if (USoundBase* UIClickSFX = UBmrSoundsDataAsset::Get().GetUIClickSFX())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), UIClickSFX);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors
void UBmrSoundsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (IS_TRANSIENT(this)
	    || !CanPlaySounds())
	{
		return;
	}

	OnBeginPlay();

	USoundMix* MainSoundMix = UBmrSoundsDataAsset::Get().GetMainSoundMix();
	UGameplayStatics::SetBaseSoundMix(&InWorld, MainSoundMix);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::Player_LocalPawnReady, this, &ThisClass::OnLocalPlayerStateReady);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	UDalRegistrySubsystem::Get().BindAndLoad<FBmrSoundsBackgroundRow>(this, &ThisClass::OnSoundRowsChanged);
}

// Is overridden to perform cleanup on ending the game
void UBmrSoundsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DestroyAllSoundComponents();

	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->Unbind(this);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called on ending the current game to play the End-Game sound
void UBmrSoundsSubsystem::OnEndGameStateChanged_Implementation(EBmrEndGameState EndGameState)
{
	if (EndGameState == EBmrEndGameState::None)
	{
		return;
	}

	if (USoundBase* EndGameSFX = UBmrSoundsDataAsset::Get().GetEndGameSFX(EndGameState))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), EndGameSFX);
	}
}

// Listen game states to switch background music
void UBmrSoundsSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		PlayInGameMusic();
		PlayStartGameCountdownSFX();
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		StopInGameMusic();
		StopStartGameCountdownSFX();
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::EndGame))
	{
		StopEndGameCountdownSFX();
	}
}

// Called when the local player state is initialized and its assigned character is ready
void UBmrSoundsSubsystem::OnLocalPlayerStateReady_Implementation(const FGameplayEventData& Payload)
{
	// Listen the ending the current game to play the End-Game sound on
	const APawn* Pawn = Cast<APawn>(Payload.Instigator.Get());
	ABmrPlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<ABmrPlayerState>() : nullptr;
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Called after background music Data Registry rows change and all new soft references finish async loading
void UBmrSoundsSubsystem::OnSoundRowsChanged_Implementation()
{
	const ABmrGameState* GameState = UBmrBlueprintFunctionLibrary::GetGameState();
	if (GameState && GameState->HasMatchingGameplayTag(FBmrGameStateTag::InGame))
	{
		PlayInGameMusic();
	}
}
