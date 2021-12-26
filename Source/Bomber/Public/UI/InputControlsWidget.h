// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"
//---
#include "InputControlsWidget.generated.h"

/**
 * Allows player to rebind input mappings.
 */
UCLASS()
class UInputControlsWidget final : public USettingCustomWidget
{
	GENERATED_BODY()

protected:
	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/* Updates appearance dynamically in the editor. */
	virtual void SynchronizeProperties() override;
};
