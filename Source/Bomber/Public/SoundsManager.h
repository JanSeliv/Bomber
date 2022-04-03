// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/AudioComponent.h"
//---
#include "Bomber.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "SoundsManager.generated.h"

/**
 * Contains all sound assets used in game that is used to manage the game sounds.
 */
UCLASS(Blueprintable, BlueprintType)
class USoundsDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the settings data asset. */
	static const USoundsDataAsset& Get();

	/** Returns the sound manager.
	 * @see USoundsDataAsset::AudioComponentClassInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class USoundsManager* GetSoundsManager() const;

	/** Returns the base Sound Mix used in game.
	 * @see USoundsDataAsset::SoundMixInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundMix* GetMainSoundMix() const { return MainSoundMixInternal; }

	/** Returns the parent class of all sounds in game.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundClass* GetMasterSoundClass() const { return MasterSoundClassInternal; }

	/** Returns the sound class of background music.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundClass* GetMusicSoundClass() const { return MusicSoundClassInternal; }

	/** Returns the sound class of the sound effects like explosions.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get SFX Sound Class"))
	FORCEINLINE class USoundClass* GetSFXSoundClass() const { return SFXSoundClassInternal; }

	/** Returns the sound of the game background theme.
	 * @see USoundsDataAsset::BackgroundCueInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundBase* GetBackgroundSound() const { return BackgroundSoundInternal; }

	/** Returns the blast SFX.
	 * @see USoundsDataAsset::ExplosionSFXInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundBase* GetExplosionSFX() const { return ExplosionSFXInternal; }

	/** Returns the sound that is played on gathering any power-up.
	 * @see USoundsDataAsset::ItemPickUpInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundBase* GetItemPickUpSFX() const { return ItemPickUpSFXInternal; }

	/** Returns the End-Game sound by specified End-Game state.
	 * @see USoundsDataAsset::EndGameSoundsInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class USoundBase* GetEndGameSFX(EEndGameState EndGameState) const;

	/** Returns the sound that is played on clicking any UI element.
	 * @see USoundsDataAsset::UIClickSFXInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get UI Click SFX"))
	FORCEINLINE class USoundBase* GetUIClickSFXInternal() const { return UIClickSFXInternal; }

protected:
	/** The Sound Manager that controls player the audio in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Sounds Manager Class", ShowOnlyInnerProperties))
	TSubclassOf<class USoundsManager> SoundsManagerClass; //[D]

	/** The base Sound Mix used in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Sound Mix", ShowOnlyInnerProperties))
	TObjectPtr<class USoundMix> MainSoundMixInternal; //[D]

	/** The parent class of all sounds in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Master Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> MasterSoundClassInternal; //[D]

	/** The sound class of background music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Music Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> MusicSoundClassInternal; //[D]

	/** The sound class of the sound effects like explosions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "SFX Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<class USoundClass> SFXSoundClassInternal; //[D]

	/** The sound of the game background theme. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Background Sound", ShowOnlyInnerProperties))
	TObjectPtr<class USoundBase> BackgroundSoundInternal; //[D]

	/** Returns the blast SFX. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Explosion Sound", ShowOnlyInnerProperties))
	TObjectPtr<class USoundBase> ExplosionSFXInternal; //[D]

	/** The sound that is played on gathering any power-up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "ItemPickUp", ShowOnlyInnerProperties))
	TObjectPtr<class USoundBase> ItemPickUpSFXInternal; //[D]

	/** Contains all sounds of End-Game states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game SFX", ShowOnlyInnerProperties))
	TMap<EEndGameState, TObjectPtr<class USoundBase>> EndGameSFXInternal; //[D]

	/** The sound that is played on clicking any UI element. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "UI Click SFX", ShowOnlyInnerProperties))
	TObjectPtr<class USoundBase> UIClickSFXInternal; //[D]

private:
	/** Is created dynamically by specified Sound Manager class.
	 * @see USoundsDataAsset::SoundManagerClass */
	UPROPERTY(Transient)
	mutable TObjectPtr<class USoundsManager> SoundManager;
};

/**
 * Is used to manage the game sounds.
 */
UCLASS(Config = "GameUserSettings", Blueprintable, BlueprintType)
class USoundsManager final : public UObject
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Returns a world of stored level map. */
	virtual class UWorld* GetWorld() const override;

	/** Set new sound volume.
	 * @param InSoundClass The class of the sounds.
	 * @param InVolume New value to set. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSoundVolumeByClass(class USoundClass* InSoundClass, float InVolume);

	/** Set the general sound volume for all sound classes in game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMasterVolume(float InVolume);

	/** Returns the general sound volume for all sound classes in game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetMasterVolume() const { return MasterVolumeInternal; }

	/** Set new sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMusicVolume(float InVolume);

	/** Returns the sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetMusicVolume() const { return MusicVolumeInternal; }

	/** Set new sound volume for SFX sound class. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DisplayName = "Set SFX Volume"))
	void SetSFXVolume(float InVolume);

	/** Returns the sound volume for SFX sound class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get SFX Volume"))
	FORCEINLINE float GetSFXVolume() const { return SFXVolumeInternal; }

	/** Play the blast sound of the bomb. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayExplosionSFX();

	/** Play the sound of the picked power-up. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void PlayItemPickUpSFX();

	/** Play the sound of the clicked UI element. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DisplayName = "Play UI Click SFX"))
	void PlayUIClickSFX();

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The general sound volume for all sound classes in game. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Master Volume"))
	float MasterVolumeInternal; //[С]

	/** The sound volume for music sound class. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Music Volume"))
	float MusicVolumeInternal; //[С]

	/** The sound volume for SFX sound class. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "SFX Volume"))
	float SFXVolumeInternal; //[С]

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
};
