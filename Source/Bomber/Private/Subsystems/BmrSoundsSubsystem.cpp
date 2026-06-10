// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrSoundsSubsystem.h"

// Bomber
#include "Bomber.h"
#include "DalRegistrySubsystem.h"
#include "DataAssets/BmrSoundsDataAsset.h"
#include "DataRegistries/BmrSoundsBackgroundRow.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "GfpmUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFeaturesSubsystem.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

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
void UBmrSoundsSubsystem::PlaySingleSound2D(USoundBase* InSound, bool bRestartIfPlaying /*= true*/)
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
	const bool bIsPlaying = SoundComponentRef.IsPlaying();

	if (bIsPlaying && !bRestartIfPlaying)
	{
		// Early return: already playing and restart not requested
		return;
	}

	if (bIsPlaying)
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

#if WITH_EDITOR
	// Release the sound and all its referenced assets: meta sounds leak in editor-only
	const UWorld* World = GetWorld();
	checkf(World, TEXT("ERROR: [%i] %hs:\n'World' is null!"), __LINE__, __FUNCTION__);
	if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
	{
		AudioDevice->Flush(nullptr);
	}
#endif

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
				UGfpmUtils::UnloadAsset(ObjectIt);
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

// Plays or stops each background music row based on world ASC tag state
void UBmrSoundsSubsystem::TryUpdateBackgroundMusic()
{
	const UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ASC)
	{
		// ASC is not ready yet, will be called again on ASC ready event
		return;
	}

	FBmrSoundsBackgroundRow::ForEachRow([&](const FBmrSoundsBackgroundRow& Row)
	{
		USoundBase* Music = Row.Music.Get();
		if (!ensureMsgf(!Row.ActivityRequirements.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'%s' row has empty 'ActivityRequirements', it will be ignored!"), __LINE__, __FUNCTION__, *Row.Music.GetAssetName())
		    || !Music)
		{
			// Music asset is not loaded yet or intentionally unloaded, will be called later when loaded
			return;
		}

		if (ASC->MatchesGameplayTagQuery(Row.ActivityRequirements))
		{
			constexpr bool bRestartIfPlaying = false;
			PlaySingleSound2D(Music, bRestartIfPlaying);
		}
		else
		{
			StopSingleSound2D(Music);
		}
	});
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

	// Apply saved volume settings from config
	SetMasterVolume(MasterVolume);
	SetMusicVolume(MusicVolume);
	SetSFXVolume(SFXVolume);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::Player_LocalPawnReady, this, &ThisClass::OnLocalPlayerStateReady);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::WorldASC_Ready, this, &ThisClass::OnWorldASCReady);

	UDalRegistrySubsystem::Get().BindAndLoad<FBmrSoundsBackgroundRow>(this, &ThisClass::OnSoundRowsChanged);

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Is overridden to cleanup injected sounds to let them unload properly
void UBmrSoundsSubsystem::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	checkf(GameFeatureData, TEXT("ERROR: [%i] %hs:\n'GameFeatureData' is null!"), __LINE__, __FUNCTION__);

	TArray<TObjectPtr<USoundBase>> SoundsOwnedByModule;
	for (const TTuple<TObjectPtr<USoundBase>, TObjectPtr<UAudioComponent>>& Pair : SoundComponents)
	{
		if (UGfpmUtils::IsInGameFeatureModule(Pair.Key, GameFeatureData))
		{
			SoundsOwnedByModule.Add(Pair.Key);
		}
	}

	// Destroy all sounds that were created by this game feature module
	for (USoundBase* Music : SoundsOwnedByModule)
	{
		DestroySingleSound2D(Music);
	}
}

// Is overridden to release world ASC tag-event bindings before world tears down
void UBmrSoundsSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	if (UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent())
	{
		ASC->RegisterGenericGameplayTagEvent().RemoveAll(this);
	}

	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	Super::OnWorldEndPlay(InWorld);
}

// Is overridden to perform cleanup on ending the game
void UBmrSoundsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DestroyAllSoundComponents();

	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->UnbindFromDataRegistryLoad(this);
	}

	DeferredBackgroundMusicUpdateHandle.Invalidate();
}

// Called on any ASC tag count change to handle tag-driven sounds
void UBmrSoundsSubsystem::OnWorldAscTagChanged_Implementation(FGameplayTag ChangedTag, int32 NewCount)
{
	if (DeferredBackgroundMusicUpdateHandle.IsValid())
	{
		// Update is already scheduled
		return;
	}

	const bool bAnyRowAffected = FBmrSoundsBackgroundRow::ContainsRowByPredicate([&ChangedTag](const FBmrSoundsBackgroundRow& Row)
	{
		TArray<FGameplayTag> OutTagDictionary;
		Row.ActivityRequirements.GetGameplayTagArray(OutTagDictionary);
		return OutTagDictionary.ContainsByPredicate([&ChangedTag](const FGameplayTag& Tag)
		{
			return ChangedTag.MatchesTag(Tag);
		});
	});

	if (!bAnyRowAffected)
	{
		// No registered row reacts to this tag, skip
		return;
	}

	// Schedule update on next tick to batch multiple tag changes that may happen in the same frame, preventing unnecessary multiple updates and music restarts
	DeferredBackgroundMusicUpdateHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
	{
		DeferredBackgroundMusicUpdateHandle.Invalidate();
		TryUpdateBackgroundMusic();
	}));
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

// Listen game states to gate start and end countdown SFX
void UBmrSoundsSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		PlayStartGameCountdownSFX();
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
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

// Called when world ASC becomes available, hooks tag-change events that drive background music rows
void UBmrSoundsSubsystem::OnWorldASCReady_Implementation(const FGameplayEventData& Payload)
{
	UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	ASC->RegisterGenericGameplayTagEvent().AddUObject(this, &ThisClass::OnWorldAscTagChanged);

	// Some tags may already be present on ASC by the time we bind, replay them
	TryUpdateBackgroundMusic();
}

// Called after background music Data Registry rows change and all new soft references finish async loading
void UBmrSoundsSubsystem::OnSoundRowsChanged_Implementation()
{
	TryUpdateBackgroundMusic();
}