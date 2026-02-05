// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"

#include "BmrSoundsDataAsset.generated.h"

class UBmrSoundsSubsystem;
class USoundBase;
class USoundClass;
class USoundMix;

enum class EBmrEndGameState : uint8;
enum class EBmrLevelType : uint8;

/**
 * Contains all sound assets used in game.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrSoundsDataAsset final : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the settings data asset. */
	static const UBmrSoundsDataAsset& Get();

	/** Returns the Sound Manager class that is responsible for audio in game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE TSubclassOf<UBmrSoundsSubsystem> GetSoundsSubsystemClass() const { return SoundsSubsystemClass; }

	/** Returns the base Sound Mix used in game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundMix* GetMainSoundMix() const { return MainSoundMix; }

	/** Returns the parent of all sounds in game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundClass* GetMasterSoundClass() const { return MasterSoundClass; }

	/** Returns the sound of background music. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundClass* GetMusicSoundClass() const { return MusicSoundClass; }

	/** Returns the sound of the sound effects like explosions. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "Get SFX Sound Class"))
	FORCEINLINE USoundClass* GetSFXSoundClass() const { return SFXSoundClass; }

	/** Returns the music of specified level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	USoundBase* GetInGameMusic(EBmrLevelType LevelType) const;

	/** Returns the blast SFX. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundBase* GetExplosionSFX() const { return ExplosionSFX; }

	/** Returns the sound that is played on gathering any power-up. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundBase* GetPowerupPickUpSFX() const { return PowerupPickUpSFX; }

	/** Returns the sound that is played right before the match ends. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundBase* GetEndGameCountdownSFX() const { return EndGameCountdownSFX; }

	/** Returns the sound that is played before the match starts. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE USoundBase* GetStartGameCountdownSFX() const { return StartGameCountdownSFX; }

	/** Returns the End-Game sound by specified End-Game state.
	 * @see UBmrSoundsDataAsset::EndGameSFX */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	USoundBase* GetEndGameSFX(EBmrEndGameState EndGameState) const;

	/** Returns the sound that is played on clicking any UI element. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "Get UI Click SFX"))
	FORCEINLINE USoundBase* GetUIClickSFX() const { return UIClickSFX; }

protected:
	/** The Sound Manager that is responsible for audio in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSubclassOf<UBmrSoundsSubsystem> SoundsSubsystemClass = nullptr;

	/** The base Sound Mix used in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundMix> MainSoundMix = nullptr;

	/** The parent of all sounds in game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> MasterSoundClass = nullptr;

	/** The sound of background music. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> MusicSoundClass = nullptr;

	/** The sound of the sound effects like explosions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundClass> SFXSoundClass = nullptr;

	/** Contains all sounds of each level to be played during the match. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TMap<EBmrLevelType, TObjectPtr<USoundBase>> InGameMusic;

	/** Returns the blast SFX. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> ExplosionSFX = nullptr;

	/** The sound that is played on gathering any power-up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> PowerupPickUpSFX = nullptr;

	/** The sound that is played right before the match ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> EndGameCountdownSFX = nullptr;

	/** The sound that is played before the match starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<USoundBase> StartGameCountdownSFX = nullptr;

	/** Contains all sounds of End-Game states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TMap<EBmrEndGameState, TObjectPtr<USoundBase>> EndGameSFX;

	/** The sound that is played on clicking any UI element. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "UI Click SFX"))
	TObjectPtr<USoundBase> UIClickSFX = nullptr;
};
