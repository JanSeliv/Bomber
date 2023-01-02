// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Components/Button.h"
//---
#include "CustomShapeButton.generated.h"

class SCustomShapeButton;
class UTexture2D;

/**
 * Unusual shape button.
 * To define a shape, use a texture with an alpha channel.
 */
UCLASS()
class CUSTOMSHAPEBUTTON_API UCustomShapeButton : public UButton
{
	GENERATED_BODY()

public:
	/** Set texture to collide with specified texture. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAdvancedHitTexture(UTexture2D* InTexture);

	/** Set new alpha. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAdvancedHitAlpha(int32 InAlpha);

protected:
	/** Texture that determines collision shape. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected))
	TObjectPtr<UTexture2D> AdvancedHitTexture = nullptr;

	/** Alpha level that determines collision shape. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, ClampMin = "0.0", ClampMax = "255.0", UIMin = "0.0", UIMax = "255.0"))
	int32 AdvancedHitAlpha = 1;

	/** Applies all properties to the native widget if possible. This is called after a widget is constructed. */
	virtual void SynchronizeProperties() override;

	/** Is called when the underlying SWidget needs to be constructed. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
