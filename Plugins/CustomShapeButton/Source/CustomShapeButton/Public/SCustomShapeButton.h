// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Widgets/Input/SButton.h"

class UTexture2D;

/**
 * Implements slate button with one difference:
 * it proceed events (hover, press) if only the mouse is on the button's non-alpha pixel.
 */
class CUSTOMSHAPEBUTTON_API SCustomShapeButton : public SButton
{
public:
	/** Virtual destructor, unregister data. */
	virtual ~SCustomShapeButton() override;

	/** Allows button to be hovered. */
	virtual void SetCanHover(bool bAllow);

protected:
	/** Cached buffer data about all pixels of current texture, is set once on render thread. */
	TSharedPtr<TArray<FColor>, ESPMode::ThreadSafe> RawColorsPtr = nullptr;

	/** Contains the size of current texture. */
	FIntPoint TextureRes = FIntPoint::ZeroValue;

	/** Is true when is allowed button to be hovered. */
	bool bCanHover = false;

	/** Contains cached true if on last frame the button was hovered. */
	bool bIsHovered = false;

	/** Contains cached geometry of the button. */
	FGeometry CurrentGeometry;

	/** Contains cached information about the mouse event. */
	FPointerEvent CurrentMouseEvent;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Returns true if cursor is hovered on a texture. */
	virtual bool IsAlphaPixelHovered() const;

	/** Set once on render thread the buffer data about all pixels of current texture if was not set before. */
	virtual void TryUpdateRawColorsOnce();

	/** Try register leaving the button (e.g. another widget opens above). */
	virtual void TickDetectMouseLeave(float DeltaTime);

	/** Try register On Hovered and On Unhovered events. */
	virtual void TryDetectOnHovered();

	/** Caching current geometry and last mouse event. */
	virtual void UpdateMouseData(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
};
