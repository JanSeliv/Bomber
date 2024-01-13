// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "SoundsDataAsset.generated.h"

class USoundsSubsystem;
class USoundBase;
class USoundClass;
class USoundMix;

enum class EEndGameState : uint8;
enum class ELevelType : uint8;
enum class ECurrentGameState : uint8;

/**
 * Contains all sound assets used in game.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USoundsDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the settings data asset. */
	static const USoundsDataAsset& Get();

	/** Returns the Sound Manager class that is responsible for audio in game.
	 * @see USoundsDataAsset::SoundsSubsystemClassInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<USoundsSubsystem> GetSoundsSubsystemClass() const { return SoundsSubsystemClassInternal; }

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

	/** Returns the music of specified level.
	 * @see USoundsDataAsset::LevelsMusicInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundBase* GetLevelMusic(ELevelType LevelType) const;

	/** Returns the main menu music of specified level.
	 * @see USoundsDataAsset::LevelsMainMenuMusicInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundBase* GetLevelMainMenuMusic(ELevelType LevelType) const;

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
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundBase* GetEndGameCountdownSFX() const { return EndGameCountdownSFXInternal; }

	/** Returns the sound that is played before the match starts.
	 * @see USoundsDataAsset::StartGameCountdownInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USoundBase* GetStartGameCountdownSFX() const { return StartGameCountdownSFXInternal; }

	/** Returns the End-Game sound by specified End-Game state.
	 * @see USoundsDataAsset::EndGameSoundsInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	USoundBase* GetEndGameSFX(EEndGameState EndGameState) const;

	/** Returns the sound that is played on clicking any UI element.
	 * @see USoundsDataAsset::UIClickSFXInternal */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get UI Click SFX"))
	FORCEINLINE USoundBase* GetUIClickSFX() const { return UIClickSFXInternal; }

protected:
	/** The Sound Manager that is responsible for audio in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Sounds Manager Class", ShowOnlyInnerProperties))
	TSubclassOf<USoundsSubsystem> SoundsSubsystemClassInternal = nullptr;

	/** The base Sound Mix used in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Sound Mix", ShowOnlyInnerProperties))
	TObjectPtr<USoundMix> MainSoundMixInternal = nullptr;

	/** The parent of all sounds in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Master Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> MasterSoundClassInternal = nullptr;

	/** The sound of background music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Music Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> MusicSoundClassInternal = nullptr;

	/** The sound of the sound effects like explosions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "SFX Sound Class", ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> SFXSoundClassInternal = nullptr;

	/** Contains all sounds of each level in the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Levels Music", ShowOnlyInnerProperties))
	TMap<ELevelType, TObjectPtr<USoundBase>> LevelsMusicInternal;

	/** Contains all sounds of each level in the main menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Levels Main Menu Music", ShowOnlyInnerProperties))
	TMap<ELevelType, TObjectPtr<USoundBase>> LevelsMainMenuMusicInternal;

	/** Returns the blast SFX. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Explosion Sound", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> ExplosionSFXInternal = nullptr;

	/** The sound that is played on gathering any power-up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Item Pick-Up SFX", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> ItemPickUpSFXInternal = nullptr;

	/** The sound that is played right before the match ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game Countdown SFX", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> EndGameCountdownSFXInternal = nullptr;

	/** The sound that is played before the match starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Start Game Countdown SFX", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> StartGameCountdownSFXInternal = nullptr;

	/** Contains all sounds of End-Game states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game SFX", ShowOnlyInnerProperties))
	TMap<EEndGameState, TObjectPtr<USoundBase>> EndGameSFXInternal;

	/** The sound that is played on clicking any UI element. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "UI Click SFX", ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> UIClickSFXInternal = nullptr;
};
