// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "NMMPlayerControllerComponent.generated.h"

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
	class AMyPlayerController* GetPlayerController() const;
	class AMyPlayerController& GetPlayerControllerChecked() const;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called. */
	virtual void OnRegister() override;
};
