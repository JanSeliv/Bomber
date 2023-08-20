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

	/** Returns HUD actor of this component. */
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

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** When setting enabled, skips previously seen cinematics automatically.
	 * Is config property, can be set in Settings menu.
	 * Note: with other players this setting is ignored and cinematics are always skipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Auto Skip Cinematics"))
	bool bAutoSkipCinematicsSettingInternal = true;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when the owning Actor begins play or when the component is created if the Actor has already begun play. */
	virtual void BeginPlay() override;

	/** Is listen to set Menu game state once first spot is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++")
	void OnMainMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);
};
