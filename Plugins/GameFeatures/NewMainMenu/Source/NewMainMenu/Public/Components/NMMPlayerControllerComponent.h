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
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
