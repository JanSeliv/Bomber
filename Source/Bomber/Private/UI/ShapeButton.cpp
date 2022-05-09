// Copyright (c) Yevhenii Selivanov.

#include "UI/ShapeButton.h"
//---
#include "UMG.h"
#include "RenderingThread.h"

// Set texture to collide with specified texture
void SShapeButton::SetAdvancedHitTexture(UTexture2D* InTexture)
{
	TextureWeakPtr = InTexture;
}

// Set new alpha
void SShapeButton::SetAdvancedHitAlpha(int32 InAlpha)
{
	AdvancedHitAlpha = InAlpha;
}

FReply SShapeButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!IsAlphaPixelHovered(MyGeometry, MouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SShapeButton::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsAlphaPixelHovered(InMyGeometry, InMouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SShapeButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!IsAlphaPixelHovered(MyGeometry, MouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SShapeButton::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bHovered = IsAlphaPixelHovered(MyGeometry, MouseEvent);
	if (bHovered != IsHovered())
	{
		SetHover(bHovered);
	}
	return SButton::OnMouseMove(MyGeometry, MouseEvent);
}

void SShapeButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SButton::OnMouseLeave(MouseEvent);

	if (IsHovered())
	{
		SetHover(false);
	}
}

FCursorReply SShapeButton::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (!IsHovered())
	{
		return FCursorReply::Unhandled();
	}
	const TOptional<EMouseCursor::Type> ThisCursor = GetCursor();
	return ThisCursor.IsSet() ? FCursorReply::Cursor(ThisCursor.GetValue()) : FCursorReply::Unhandled();
}

TSharedPtr<IToolTip> SShapeButton::GetToolTip()
{
	return IsHovered() ? SWidget::GetToolTip() : nullptr;
}

// Returns true if cursor is hovered on a texture
bool SShapeButton::IsAlphaPixelHovered(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	const bool bIsCurrentlyHovered = IsHovered();

	if (!AdvancedHitAlpha)
	{
		return bIsCurrentlyHovered;
	}

	const UTexture2D* HitTexture = TextureWeakPtr.Get();
	if (!HitTexture)
	{
		return bIsCurrentlyHovered;
	}

	const uint32 SizeX = HitTexture->GetSizeX();
	const uint32 SizeY = HitTexture->GetSizeY();

	struct FRawColorsData
	{
		TArray<FColor> RawColors;
		TPromise<bool> RawColorsCopied{nullptr};
	};

	TSharedPtr<FRawColorsData, ESPMode::ThreadSafe> RawColorsDataPtr = MakeShared<FRawColorsData, ESPMode::ThreadSafe>();
	RawColorsDataPtr->RawColors.SetNum(SizeX * SizeY);

	// Get Raw Colors data on Render thread
	ENQUEUE_RENDER_COMMAND(NeedExecuteAction)([RawColorsDataPtr, WeakTexture = TextureWeakPtr](FRHICommandListImmediate&)
	{
		FRawColorsData* RawColorsData = RawColorsDataPtr.Get();
		check(RawColorsData);

		const UTexture2D* Texture2D = WeakTexture.Get();
		const FTextureResource* TextureResource = Texture2D ? Texture2D->GetResource() : nullptr;
		FRHITexture2D* RHITexture2D = TextureResource ? TextureResource->GetTexture2DRHI() : nullptr;
		check(RHITexture2D);

		// Lock		
		uint32 DestPitch = 0;
		constexpr int32 MipIndex = 0;
		constexpr bool bLockWithinMipTail = false;
		const uint8* MappedTextureMemory = static_cast<const uint8*>(RHILockTexture2D(RHITexture2D, MipIndex, RLM_ReadOnly, DestPitch, bLockWithinMipTail));

		// Copy data
		TArray<FColor>& RawColorsRef = RawColorsData->RawColors;
		const int32 Count = RawColorsRef.Num() * sizeof(FColor);
		FMemory::Memcpy(/*dest*/RawColorsRef.GetData(), /*source*/MappedTextureMemory, Count);

		// Unlock
		RHIUnlockTexture2D(RHITexture2D, MipIndex, bLockWithinMipTail);

		// Notify
		RawColorsDataPtr->RawColorsCopied.SetValue(true);
	});

	const bool bRawColorsCopied = RawColorsDataPtr->RawColorsCopied.GetFuture().Get();
	if (!bRawColorsCopied)
	{
		return bIsCurrentlyHovered;
	}

	FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	LocalPosition.X = FMath::Floor(LocalPosition.X);
	LocalPosition.Y = FMath::Floor(LocalPosition.Y);
	LocalPosition /= MyGeometry.GetLocalSize();
	LocalPosition.X *= SizeX;
	LocalPosition.Y *= SizeY;
	const int32 BufferPosition = FMath::Floor(LocalPosition.Y) * SizeX + LocalPosition.X;

	const TArray<FColor>& RawColors = RawColorsDataPtr->RawColors;
	if (!RawColors.IsValidIndex(BufferPosition))
	{
		return bIsCurrentlyHovered;
	}

	const bool bIsAlphaPixelHovered = RawColors[BufferPosition].A > AdvancedHitAlpha;
	return bIsAlphaPixelHovered;
}

// Set texture to collide with specified texture
void UShapeButton::SetAdvancedHitTexture(UTexture2D* InTexture)
{
	SButton* MyButtonPtr = MyButton.Get();
	if (!MyButtonPtr
	    || !InTexture)
	{
		return;
	}

	AdvancedHitTexture = InTexture;
	if (SShapeButton* ShapeButton = static_cast<SShapeButton*>(MyButtonPtr))
	{
		ShapeButton->SetAdvancedHitTexture(AdvancedHitTexture);
	}
}

// Set new alpha
void UShapeButton::SetAdvancedHitAlpha(int32 InAlpha)
{
	SButton* MyButtonPtr = MyButton.Get();
	if (!MyButtonPtr)
	{
		return;
	}

	AdvancedHitAlpha = InAlpha;
	if (SShapeButton* ShapeButton = static_cast<SShapeButton*>(MyButtonPtr))
	{
		ShapeButton->SetAdvancedHitAlpha(AdvancedHitAlpha);
	}
}

// Applies all properties to the native widget if possible
void UShapeButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	SetAdvancedHitTexture(AdvancedHitTexture);
	SetAdvancedHitAlpha(AdvancedHitAlpha);
}

// Is called when the underlying SWidget needs to be constructed
TSharedRef<SWidget> UShapeButton::RebuildWidget()
{
	const TSharedRef<SShapeButton> NewButtonRef = SNew(SShapeButton)
											 .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
											 .OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
											 .OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
											 .OnHovered_UObject(this, &ThisClass::SlateHandleHovered)
											 .OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhovered)
											 .ButtonStyle(&WidgetStyle)
											 .ClickMethod(ClickMethod)
											 .TouchMethod(TouchMethod)
											 .IsFocusable(IsFocusable);
	MyButton = NewButtonRef;

	NewButtonRef->SetAdvancedHitTexture(AdvancedHitTexture);
	NewButtonRef->SetAdvancedHitAlpha(AdvancedHitAlpha);

	if (GetChildrenCount())
	{
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(GetContentSlot()))
		{
			ButtonSlot->BuildSlot(NewButtonRef);
		}
	}

	return NewButtonRef;
}
