// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/WidgetInteractionComponent.h"
//---
#include "Bomber.h"
//---
#include "MyWidgetInteractionComponent.generated.h"

/**
 * Custom Widget Interaction Component.
 * used to be able press buttons on 3D widget placed in world.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMyWidgetInteractionComponent final : public UWidgetInteractionComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UMyWidgetInteractionComponent();

	/** Sets most suitable Virtual User index by current player index. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdatePlayerIndex();

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Listen game states to manage the enabling and disabling this component. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Pushes the owner actor to the stack of input to be able to send input key events. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void EnableInput();

	/** Sends the press key event to slate, is overriden to ignore if component is not active. */
	virtual bool PressKey(FKey Key, bool bRepeat) override;

	/** Sends the release key event to slate, is overriden to ignore if component is not active. */
	virtual bool ReleaseKey(FKey Key) override;
};
