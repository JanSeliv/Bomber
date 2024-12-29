// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "NMMHUDComponent.generated.h"

/**
 * Represents the HUD in the NewMain Menu module, where the Owner is HUD actor.
 * Is responsible for managing Main Menu widgets.
 */
UCLASS(BlueprintType, Blueprintable, DisplayName = "NNM HUD Component", ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNMMHUDComponent : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Default constructor. */
	UNMMHUDComponent();

	/** Returns created Main Menu widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNewMainMenuWidget* GetMainMenuWidget() const { return MainMenuWidgetInternal; }

	/** Returns created In Cinematic State widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNMMCinematicStateWidget* GetInCinematicStateWidget() const { return InCinematicStateWidgetInternal; }

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Created Main Menu widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main Menu Widget"))
	TObjectPtr<class UNewMainMenuWidget> MainMenuWidgetInternal = nullptr;

	/** Created In Cinematic State widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "In Cinematic State Widget"))
	TObjectPtr<class UNMMCinematicStateWidget> InCinematicStateWidgetInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called. */
	virtual void OnRegister() override;

	/** Clears all transient data created by this component. */
	virtual void OnUnregister() override;

	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnLocalCharacterReady(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};
