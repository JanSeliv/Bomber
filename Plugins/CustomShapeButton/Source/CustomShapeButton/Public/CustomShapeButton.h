// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Components/Button.h"
//---
#include "CustomShapeButton.generated.h"

class SCustomShapeButton;

/**
 * Custom shape button.
 * To make it work:
 * - ave an image with alpha pixels
 * - set an image as for regular button under the 'Appearance' -> 'Style' category
 * Is wrapper around the SCustomShapeButton widget.
 * @see SCustomShapeButton
 */
UCLASS()
class CUSTOMSHAPEBUTTON_API UCustomShapeButton : public UButton
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UCustomShapeButton();

	/** Returns the slate shape button. */
	TSharedPtr<SCustomShapeButton> GetSlateCustomShapeButton() const;

protected:
	/** Is called when the underlying SWidget needs to be constructed. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
