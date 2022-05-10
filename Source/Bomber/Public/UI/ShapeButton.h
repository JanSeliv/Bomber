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

protected:
	/** Current texture to determine collision shape. */
	TWeakObjectPtr<class UTexture2D> TextureWeakPtr = nullptr;

	/** Current alpha. */
	int32 AdvancedHitAlpha = 1;

	/** Cached buffer data about all pixels of current texture, is set once on render thread. */
	TSharedPtr<TArray<FColor>, ESPMode::ThreadSafe> RawColorsPtr = nullptr;

	/** Contains the size of current texture. */
	FIntPoint TextureRes = FIntPoint::ZeroValue;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual TSharedPtr<IToolTip> GetToolTip() override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Returns true if cursor is hovered on a texture. */
	bool IsAlphaPixelHovered(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	/** Set once on render thread the buffer data about all pixels of current texture if was not set before. */
	void TryUpdateRawColorsOnce();
};

/**
 * Unusual shape button.
 * To define a shape, use a texture with an alpha channel.
 */
UCLASS()
class UShapeButton final : public UButton
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
