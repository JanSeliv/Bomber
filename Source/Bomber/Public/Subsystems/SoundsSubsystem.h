// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "SoundsSubsystem.generated.h"

enum class ELevelType : uint8;
enum class ECurrentGameState : uint8;
enum class EEndGameState : uint8;

/**
 * Is used to manage the game sounds.
 * @see Access its data with USoundsDataAsset (Content/Bomber/DataAssets/DA_Sounds).
 */
UCLASS(Config = "GameUserSettings", DefaultConfig, Blueprintable, BlueprintType)
class BOMBER_API USoundsSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*********************************************************************************************
	 * Static methods
	 ********************************************************************************************* */
public:
	/** Returns the Sounds Manager, is checked and wil crash if can't be obtained. */
	static USoundsSubsystem& Get();

	/** Returns the pointer to the Sounds Manager. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static USoundsSubsystem* GetSoundsSubsystem(const UObject* WorldContextObject = nullptr);

	/** Static method that returns true if sounds can be played. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool CanPlaySounds();

	/*********************************************************************************************
	 * Single sound
	 * Is expanded to be used by all places in project to revent concurrency (multiple sounds playing at the same time).
	 ********************************************************************************************* */
public:
	/** Play the sound in 2D space with ensuring that this sound component is created only once.
	 * If component is already created, it will use existing one.
	 * If sound itself is already playing, it will stop existing one and play new one.
	 * @warning it does not return UAudioComponent since it's managed internally, call USoundSubsystem::Get().StopSound2D(Sound) to stop the sound. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlaySingleSound2D(class USoundBase* InSound);

	/** Deactivates the given sound if currently playing. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopSingleSound2D(class USoundBase* InSound);

	/** Destroy sound component by given sound, it's used to perform cleanup when game is finished. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DestroySingleSound2D(class USoundBase* InSound);

protected:
	/** All known sound components that are playing single sounds. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Sound Components"))
	TMap<TObjectPtr<USoundBase>, TObjectPtr<class UAudioComponent>> SoundComponentsInternal;

	/** Performs cleanup on all known sound components. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void DestroyAllSoundComponents();

	/*********************************************************************************************
	 * Volume
	 ********************************************************************************************* */
public:
	/** Set new sound volume.
	 * @param InSoundClass The of the sounds.
	 * @param InVolume New value to set. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSoundVolumeByClass(class USoundClass* InSoundClass, float InVolume);

	/** Set the general sound volume for all sound classes in game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMasterVolume(double InVolume);

	/** Returns the general sound volume for all sound classes in game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE double GetMasterVolume() const { return MasterVolumeInternal; }

	/** Set new sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMusicVolume(double InVolume);

	/** Returns the sound volume for music sound class. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE double GetMusicVolume() const { return MusicVolumeInternal; }

	/** Set new sound volume for SFX sound class. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DisplayName = "Set SFX Volume"))
	void SetSFXVolume(double InVolume);

	/** Returns the sound volume for SFX sound class. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get SFX Volume"))
	FORCEINLINE double GetSFXVolume() const { return SFXVolumeInternal; }

protected:
	/** The general sound volume for all sound classes in game, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Master Volume"))
	double MasterVolumeInternal;

	/** The sound volume for music sound class, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Music Volume"))
	double MusicVolumeInternal;

	/** The sound volume for SFX sound class, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "SFX Volume"))
	double SFXVolumeInternal;

	/*********************************************************************************************
	 * Public API
	 ********************************************************************************************* */
public:
	/** Trigger the background music to be played during the match. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayInGameMusic();

	/** Stops currently played in-match background music. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopInGameMusic();

	/** Play the blast sound of the bomb. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayExplosionSFX();

	/** Play the sound of the picked power-up. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayItemPickUpSFX();

	/** Play the sound that is played right before the match ends. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayEndGameCountdownSFX();

	/** Stops the sound that is played right before the match ends. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopEndGameCountdownSFX();

	/** Play the sound that is played before the match starts. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayStartGameCountdownSFX();

	/** Stops the sound that is played right after the match starts. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopStartGameCountdownSFX();

	/** Play the sound of the clicked UI element. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DisplayName = "Play UI Click SFX"))
	void PlayUIClickSFX();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Is overridden to perform cleanup on ending the game. */
	virtual void Deinitialize() override;

	/** Blueprint even called when the game starts. */
	UFUNCTION(BlueprintImplementableEvent, Category = "C++", meta = (DisplayName = "Begin Play"))
	void OnBeginPlay();

	/** Is called to play the End-Game sound on ending the current game. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EEndGameState EndGameState);

	/** Listen game states to switch background music. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when the local player state is initialized and its assigned character is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnLocalPlayerStateReady(class AMyPlayerState* PlayerState, int32 CharacterID);
};