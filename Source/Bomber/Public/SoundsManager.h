// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Components/AudioComponent.h"
//---
#include "Globals/LevelActorDataAsset.h"
//---
#include "SoundsManager.generated.h"

/**
 * Contains all sound assets used in game.
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
	FORCEINLINE USoundClass* GetMasterSoundClass() const { return MasterSoundClassInternal; }

	/** Returns the sound class of background music.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE USoundClass* GetMusicSoundClass() const { return MusicSoundClassInternal; }

	/** Returns the sound class of the sound effects like explosions.
	 * @see USoundsDataAsset::MasterSoundClassInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get SFX Sound Class"))
	FORCEINLINE USoundClass* GetSFXSoundClass() const { return SFXSoundClassInternal; }

	/** Returns the sound of the game background theme.
	 * @see USoundsDataAsset::BackgroundCueInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USoundBase* GetBackgroundSound() const { return BackgroundSoundInternal; }

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
	/** Returns a world of stored level map. */
	virtual class UWorld* GetWorld() const override;

	/** Set new sound volume.
	 * @param InSoundClass The class of the sounds.
	 * @param InVolume New value to set. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSoundVolumeByClass(class USoundClass* InSoundClass, float InVolume);

	/** Set new sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMusicVolume(float InVolume);

	/** Returns the sound volume for music sound class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetMusicVolume() const { return MusicVolumeInternal; }

protected:
	/** Called after the C++ constructor and after the properties have been initialized, including those loaded from config.*/
	virtual void PostInitProperties() override;

	/** Called when the game starts. */
	void BeginPlay();

	UFUNCTION(BlueprintImplementableEvent, Category = "C++", meta = (DisplayName = "Begin Play"))
	void OnBeginPlay();

	/** The sound volume for music sound class. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Music Volume"))
	float MusicVolumeInternal; //[С]
};
