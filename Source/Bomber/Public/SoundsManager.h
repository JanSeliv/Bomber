// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/AudioComponent.h"
//---
#include "Bomber.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "SoundsManager.generated.h"

class USoundsManager;
class USoundBase;
class USoundClass;

/**
 * Contains all sound assets used in game.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USoundsDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the settings data asset. */
	static const USoundsDataAsset& Get();

	/** Returns the sound manager.
	 * @see USoundsDataAsset::AudioComponentClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundsManager* GetSoundsManager() const;

	/** Returns the base Sound Mix used in game.
	 * @see USoundsDataAsset::SoundMixInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundMix* GetMainSoundMix() const { return MainSoundMixInternal; }

	/** Returns the parent of all sounds in game.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundClass* GetMasterSoundClass() const { return MasterSoundClassInternal; }

	/** Returns the sound of background music.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundClass* GetMusicSoundClass() const { return MusicSoundClassInternal; }

	/** Returns the sound of the sound effects like explosions.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get SFX Sound Class"))
	FORCEINLINE USoundClass* GetSFXSoundClass() const { return SFXSoundClassInternal; }

	/** Returns the music of the Main-Menu theme.
	 * @see USoundsDataAsset::MainMenuMusicInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundBase* GetMainMenuMusic() const { return MainMenuMusicInternal; }

	/** Returns the music of specified level.
	 * @see USoundsDataAsset::LevelsMusicInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundBase* GetLevelMusic(ELevelType LevelType) const;

	/** Return the background music by specified game state and level type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundBase* GetBackgroundMusic(ECurrentGameState CurrentGameState, ELevelType LevelType) const;

	/** Returns the blast SFX.
	 * @see USoundsDataAsset::ExplosionSFXInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundBase* GetExplosionSFX() const { return ExplosionSFXInternal; }

	/** Returns the sound that is played on gathering any power-up.
	 * @see USoundsDataAsset::ItemPickUpInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundBase* GetItemPickUpSFX() const { return ItemPickUpSFXInternal; }

	/** Returns the sound that is played right before the match ends.
	 * @see USoundsDataAsset::EndGameCountdownInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundBase* GetEndGameCountdownSFX() const { return EndGameCountdownSFXInternal; }

	/** Returns the End-Game sound by specified End-Game state.
	 * @see USoundsDataAsset::EndGameSoundsInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundBase* GetEndGameSFX(EEndGameState EndGameState) const;

	/** Returns the sound that is played on clicking any UI element.
	 * @see USoundsDataAsset::UIClickSFXInternal */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get UI Click SFX"))
	FORCEINLINE USoundBase* GetUIClickSFX() const { return UIClickSFXInternal; }

protected:
	/** The Sound Manager that controls player the audio in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Sounds Manager Class", ShowOnlyInnerProperties))
	TSubclassOf<USoundsManager> SoundsManagerClass = nullptr; //[D]

	/** The base Sound Mix used in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Sound Mix", ShowOnlyInnerProperties))
	TObjectPtr<USoundMix> MainSoundMixInternal = nullptr; //[D]

	/** The parent of all sounds in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Master Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> MasterSoundClassInternal = nullptr; //[D]

	/** The sound of background music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Music Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> MusicSoundClassInternal = nullptr; //[D]

	/** The sound of the sound effects like explosions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "SFX Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> SFXSoundClassInternal = nullptr; //[D]

	/** The sound of the game background theme. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main-Menu Music", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> MainMenuMusicInternal = nullptr; //[D]

	/** Contains all sounds of each level in the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Levels Music", ShowOnlyInnerProperties))
	TMap<ELevelType, TObjectPtr<USoundBase>> LevelsMusicInternal; //[D]

	/** Returns the blast SFX. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Explosion Sound", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> ExplosionSFXInternal = nullptr; //[D]
	
	/** The sound that is played on gathering any power-up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Item Pick-Up SFX", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> ItemPickUpSFXInternal = nullptr; //[D]

	/** The sound that is played right before the match ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "EndGame Countdown SFX ", ShowOnlyInnerProperties))
	TObjectPtr<class USoundBase> EndGameCountdownSFXInternal = nullptr; //[D]
	

	/** Contains all sounds of End-Game states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game SFX", ShowOnlyInnerProperties))
	TMap<EEndGameState, TObjectPtr<USoundBase>> EndGameSFXInternal; //[D]

	/** The sound that is played on clicking any UI element. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "UI Click SFX", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> UIClickSFXInternal = nullptr; //[D]

private:
	/** Is created dynamically by specified Sound Manager class.
	 * @see USoundsDataAsset::SoundsManagerClass */
	UPROPERTY(Transient)
	mutable TObjectPtr<USoundsManager> SoundsManager = nullptr;
};

/**
 * Is used to manage the game sounds.
 */
UCLASS(Config = "GameUserSettings", Blueprintable, BlueprintType)
class BOMBER_API USoundsManager final : public UObject
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Returns a world of stored level map. */
	virtual UWorld* GetWorld() const override;

	/** Set new sound volume.
	 * @param InSoundClass The of the sounds.
	 * @param InVolume New value to set. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSoundVolumeByClass(USoundClass* InSoundClass, float InVolume);

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

	/** The general sound volume for all sound classes in game. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Master Volume"))
	double MasterVolumeInternal; //[С]

	/** The sound volume for music sound class. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Music Volume"))
	double MusicVolumeInternal; //[С]

	/** The sound volume for SFX sound class. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "SFX Volume"))
	double SFXVolumeInternal; //[С]

	/** The component that is used to play different background musics.  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Background Music Component"))
	TObjectPtr<UAudioComponent> BackgroundMusicComponentInternal = nullptr; //[G]

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
