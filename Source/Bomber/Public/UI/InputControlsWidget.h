// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"
//---
#include "InputControlsWidget.generated.h"

/**
 * Allows player to rebind input mappings.
 */
UCLASS()
class BOMBER_API UInputControlsWidget final : public USettingCustomWidget
{
	GENERATED_BODY()

public:
	/** Sets the style of the button and its text. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "TextStyle,ButtonStyle"))
	void SetStyle(class UInputKeySelector* InputKeySelector, const FTextBlockStyle& TextStyle, const FButtonStyle& ButtonStyle);

protected:
	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/* Updates appearance dynamically in the editor. */
	virtual void SynchronizeProperties() override;
};
