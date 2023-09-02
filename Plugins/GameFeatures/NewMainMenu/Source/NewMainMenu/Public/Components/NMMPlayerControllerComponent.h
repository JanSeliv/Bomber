// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "NMMPlayerControllerComponent.generated.h"

class AMyPlayerController;

/**
 * Represents the Player Controller in the NewMain Menu module, where the Owner is Player Controller actor.
 * Is responsible for managing Main Menu inputs.
 */
UCLASS(Config = "GameUserSettings", DefaultConfig, Blueprintable, BlueprintType, DisplayName = "New Main Menu Player Controller Component", ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNMMPlayerControllerComponent : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Default constructor. */
	UNMMPlayerControllerComponent();

	/** Returns Player Controller of this component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	AMyPlayerController* GetPlayerController() const;
	AMyPlayerController& GetPlayerControllerChecked() const;

	/** Returns true is setting enabled to skips previously seen cinematics automatically.
	 * @see UNMMPlayerControllerComponent::bAutoSkipCinematicsInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsAutoSkipCinematicsSetting() const { return bAutoSkipCinematicsSettingInternal; }

	/** Set true to skip previously seen cinematics automatically.
	 * Is called from Settings menu once its checkbox is changed. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAutoSkipCinematicsSetting(bool bEnable);

	/** Returns loaded and cached Save Game Data of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNMMSaveGameData* GetSaveGameData() const { return SaveGameDataInternal; }

	/** Removes all saved data of the Main Menu and creates new empty data. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ResetSaveGameData();

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** When setting enabled, skips previously seen cinematics automatically.
	 * Is config property, can be set in Settings menu.
	 * Note: with other players this setting is ignored and cinematics are always skipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Auto Skip Cinematics"))
	bool bAutoSkipCinematicsSettingInternal = true;

	/** Contains loaded and cached Save Game Data of the Main Menu. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Save Game Data"))
	TObjectPtr<class UNMMSaveGameData> SaveGameDataInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when the owning Actor begins play or when the component is created if the Actor has already begun play. */
	virtual void BeginPlay() override;

	/** Is listen to set Menu game state once first spot is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMainMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);

	/** Is called from AsyncLoadGameFromSlot once Save Game is loaded, or null if it failed to load. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAsyncLoadGameFromSlotCompleted(const FString& SlotName, int32 UserIndex, class USaveGame* SaveGame);
};
