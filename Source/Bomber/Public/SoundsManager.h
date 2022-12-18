// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/AudioComponent.h"
//---
#include "Bomber.h"
//---
#include "SoundsManager.generated.h"

/**
 * Is used to manage the game sounds.
 * @see Access its data with USoundsDataAsset (Content/Bomber/Globals/DA_Sounds).
 */
UCLASS(Config = "GameUserSettings", Blueprintable, BlueprintType)
class BOMBER_API USoundsManager final : public UObject
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Returns the Sounds Manager checked. */
	static USoundsManager& Get();

	/** Returns a world of stored level map. */
	virtual UWorld* GetWorld() const override;

	/** The component that is used to store reference for EndGameCountdown SFX.*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Background Music Component"))
	TObjectPtr<UAudioComponent> EndGameCountDownSFXAudioComponent = nullptr;

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

	/** Play the background music for current game state and level. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayCurrentBackgroundMusic();

	/** Play the blast sound of the bomb. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayExplosionSFX();

	/** Play the sound of the picked power-up. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayItemPickUpSFX();

	/** Play the sound that is played right before the match ends. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayEndGameCountdownSFX();

	/** Play the sound of the clicked UI element. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DisplayName = "Play UI Click SFX"))
	void PlayUIClickSFX();

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The general sound volume for all sound classes in game, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Master Volume"))
	double MasterVolumeInternal;

	/** The sound volume for music sound class, is config property, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Music Volume"))
	double MusicVolumeInternal;

	/** The sound volume for SFX sound class, is config property, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "SFX Volume"))
	double SFXVolumeInternal;

	/** The component that is used to play different background musics.  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Background Music Component"))
	TObjectPtr<UAudioComponent> BackgroundMusicComponentInternal = nullptr;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called after the C++ constructor and after the properties have been initialized, including those loaded from config.*/
	virtual void PostInitProperties() override;

	/** Called when the game starts. */
	void BeginPlay();

	/** Blueprint even called when the game starts. */
	UFUNCTION(BlueprintImplementableEvent, Category = "C++", meta = (DisplayName = "Begin Play"))
	void OnBeginPlay();

	/** Is called to play the End-Game sound on ending the current game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EEndGameState EndGameState);

	/** Listen game states to switch background music. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
