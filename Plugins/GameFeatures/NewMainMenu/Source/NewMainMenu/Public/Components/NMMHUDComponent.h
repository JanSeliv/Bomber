// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

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

	/** Returns the Main Menu widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	class UNewMainMenuWidget* GetMainMenuWidget() const;

	/** Returns the In Cinematic State widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	class UNMMCinematicStateWidget* GetInCinematicStateWidget() const;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called. */
	virtual void OnRegister() override;

	/** Clears all transient data created by this component. */
	virtual void OnUnregister() override;

	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnLocalPawnReady(const struct FGameplayEventData& Payload);
};
