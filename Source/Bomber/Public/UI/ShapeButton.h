// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Components/Button.h"
#include "ShapeButton.generated.h"

/**
 * Implemented slate button to allow player interact with custom shape that is set by texture.
 */
class SShapeButton final : public SButton
{
public:
	virtual ~SShapeButton() override;

	/** Set texture to collide with specified texture. */
	void SetAdvancedHitTexture(class UTexture2D* InTexture);

	/** Set new alpha. */
	void SetAdvancedHitAlpha(int32 InAlpha);

	/** Allows button to be hovered. */
	void SetCanHover(bool bAllow);

protected:
	/** Current texture to determine collision shape. */
	TWeakObjectPtr<class UTexture2D> TextureWeakPtr = nullptr;

	/** Current alpha. */
	int32 AdvancedHitAlpha = 1;

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
	bool IsAlphaPixelHovered() const;

	/** Set once on render thread the buffer data about all pixels of current texture if was not set before. */
	void TryUpdateRawColorsOnce();

	/** Try register leaving the button (e.g. another widget opens above). */
	void TickDetectMouseLeave(float DeltaTime);

	/** Try register On Hovered and On Unhovered events. */
	void TryDetectOnHovered();

	/** Caching current geometry and last mouse event. */
	void UpdateMouseData(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
};

/**
 * Unusual shape button.
 * To define a shape, use a texture with an alpha channel.
 */
UCLASS()
class BOMBER_API UShapeButton final : public UButton
{
	GENERATED_BODY()

public:
	/** Set texture to collide with specified texture. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAdvancedHitTexture(class UTexture2D* InTexture);

	/** Set new alpha. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAdvancedHitAlpha(int32 InAlpha);

protected:
	/** Texture that determines collision shape. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected))
	TObjectPtr<class UTexture2D> AdvancedHitTexture = nullptr;

	/** Alpha level that determines collision shape. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, ClampMin = "0.0", ClampMax = "255.0", UIMin = "0.0", UIMax = "255.0"))
	int32 AdvancedHitAlpha = 1;

	/** Applies all properties to the native widget if possible. This is called after a widget is constructed. */
	virtual void SynchronizeProperties() override;

	/** Is called when the underlying SWidget needs to be constructed. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
