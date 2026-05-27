// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// UE
#include "GameFeatureStateChangeObserver.h"

#include "BmrSoundsSubsystem.generated.h"

enum class EBmrEndGameState : uint8;

/**
 * Is used to manage the game sounds.
 * @see Access its data with UBmrSoundsDataAsset (Content/Bomber/DataAssets/DA_Sounds).
 */
UCLASS(Config = "GameUserSettings", DefaultConfig, Blueprintable, BlueprintType)
class BOMBER_API UBmrSoundsSubsystem final : public UWorldSubsystem
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/*********************************************************************************************
	 * Static methods
	 ********************************************************************************************* */
public:
	/** Returns the Sounds Manager, is checked and wil crash if can't be obtained. */
	static UBmrSoundsSubsystem& Get();

	/** Returns the pointer to the Sounds Manager. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "WorldContextObject"))
	static UBmrSoundsSubsystem* GetSoundsSubsystem(const UObject* WorldContextObject = nullptr);

	/** Static method that returns true if sounds can be played. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static bool CanPlaySounds();

	/*********************************************************************************************
	 * Single sound
	 * Is expanded to be used by all places in project to revent concurrency (multiple sounds playing at the same time).
	 ********************************************************************************************* */
public:
	/** Play the sound in 2D space with ensuring that this sound component is created only once.
	 * If component is already created, it will use existing one.
	 * @param InSound - Sound asset to play.
	 * @param bRestartIfPlaying - If true and sound itself is already playing, it will stop existing one and play new one.
	 * @warning it does not return UAudioComponent since it's managed internally, call UBmrSoundsSubs::Get().StopSound2D(Sound) to stop the sound. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void PlaySingleSound2D(class USoundBase* InSound, bool bRestartIfPlaying = true);

	/** Deactivates the given sound if currently playing. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void StopSingleSound2D(class USoundBase* InSound);

	/** Destroy sound component by given sound, it's used to perform cleanup when game is finished. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void DestroySingleSound2D(class USoundBase* InSound);

protected:
	/** All known sound components that are playing single sounds. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TMap<TObjectPtr<USoundBase>, TObjectPtr<class UAudioComponent>> SoundComponents;

	/** Performs cleanup on all known sound components. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void DestroyAllSoundComponents();

	/*********************************************************************************************
	 * Volume
	 ********************************************************************************************* */
public:
	/** Set new sound volume.
	 * @param InSoundClass The of the sounds.
	 * @param InVolume New value to set. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetSoundVolumeByClass(class USoundClass* InSoundClass, float InVolume);

	/** Set the general sound volume for all sound classes in game. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMasterVolume(double InVolume);

	/** Returns the general sound volume for all sound classes in game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE double GetMasterVolume() const { return MasterVolume; }

	/** Set new sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetMusicVolume(double InVolume);

	/** Returns the sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE double GetMusicVolume() const { return MusicVolume; }

	/** Set new sound volume for SFX sound class. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DisplayName = "Set SFX Volume"))
	void SetSFXVolume(double InVolume);

	/** Returns the sound volume for SFX sound class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "Get SFX Volume"))
	FORCEINLINE double GetSFXVolume() const { return SFXVolume; }

protected:
	/** The general sound volume for all sound classes in game, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	double MasterVolume;

	/** The sound volume for music sound class, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	double MusicVolume;

	/** The sound volume for SFX sound class, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	double SFXVolume;

	/*********************************************************************************************
	 * Public API
	 ********************************************************************************************* */
public:
	/** Play the sound that is played right before the match ends. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void PlayEndGameCountdownSFX();

	/** Stops the sound that is played right before the match ends. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void StopEndGameCountdownSFX();

	/** Play the sound that is played before the match starts. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void PlayStartGameCountdownSFX();

	/** Stops the sound that is played right after the match starts. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void StopStartGameCountdownSFX();

	/** Play the sound of the clicked UI element. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DisplayName = "Play UI Click SFX"))
	void PlayUIClickSFX();

	/*********************************************************************************************
	 * Background Music
	 ********************************************************************************************* */
public:
	/** Plays or stops each background music row based on world ASC tag state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TryUpdateBackgroundMusic();

	/** Handle for deferred background music recompute, prevents burst tag events from restarting tracks mid-transition. */
	FTimerHandle DeferredBackgroundMusicUpdateHandle;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Is overridden to cleanup injected sounds to let them unload properly. */
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override;

	/** Called when world ends play. */
	virtual void OnWorldEndPlay(UWorld& InWorld) override;

	/** Is overridden to perform cleanup on ending the game. */
	virtual void Deinitialize() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Blueprint even called when the game starts. */
	UFUNCTION(BlueprintImplementableEvent, Category = "[Bomber]", meta = (DisplayName = "Begin Play"))
	void OnBeginPlay();

	/** Is called to play the End-Game sound on ending the current game. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EBmrEndGameState EndGameState);

	/** Listen game states to gate start and end countdown SFX. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Called when the local player state is initialized and its assigned character is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnLocalPlayerStateReady(const struct FGameplayEventData& Payload);

	/** Called when world ASC becomes available. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnWorldASCReady(const struct FGameplayEventData& Payload);

	/** Called on any ASC tag count change to handle tag-driven sounds. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnWorldAscTagChanged(struct FGameplayTag ChangedTag, int32 NewCount);

	/** Called after background music Data Registry rows change and all new soft references finish async loading. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnSoundRowsChanged();
};