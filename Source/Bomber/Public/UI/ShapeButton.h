// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Components/Button.h"
#include "ShapeButton.generated.h"

/**
 *
 */
class SShapeButton final : public SButton
{
public:
	/** */
	void SetAdvancedHitTexture(class UTexture2D* InTexture);

	/**  */
	void SetAdvancedHitAlpha(int32 InAlpha);

protected:
	/**  */
	class UTexture2D* AdvancedHitTexture = nullptr;

	/**  */
	int32 AdvancedHitAlpha = 1;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual TSharedPtr<IToolTip> GetToolTip() override;

	/**  */
	bool NeedExecuteAction(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
};


/**
 * Unusual shape button
 * To define a shape, use a texture with an alpha channel
 */
UCLASS()
class BOMBER_API UShapeButton final : public UButton
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	explicit UShapeButton(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAdvancedHitTexture(class UTexture2D* InTexture);

	/** */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAdvancedHitAlpha(int32 InAlpha);

protected:
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected))
	class UTexture2D* AdvancedHitTexture = nullptr;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, ClampMin = "0.0", ClampMax = "255.0", UIMin = "0.0", UIMax = "255.0"))
	int32 AdvancedHitAlpha = 1;

	/** */
	virtual void SynchronizeProperties() override;

	/** */
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
