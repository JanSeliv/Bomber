// Copyright 2021 Yevhenii Selivanov.

#include "UI/ShapeButton.h"
//---
#include "UMG.h"

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
	if (!NeedExecuteAction(MyGeometry, MouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SShapeButton::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (!NeedExecuteAction(InMyGeometry, InMouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SShapeButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!NeedExecuteAction(MyGeometry, MouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SShapeButton::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bHovered = NeedExecuteAction(MyGeometry, MouseEvent);
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

// Called on mouse moves and clicks
bool SShapeButton::NeedExecuteAction(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	// Return true by default do not prevent player press the button
	bool bSucceed = true;

	if (!AdvancedHitAlpha)
	{
		return bSucceed;
	}

	UTexture2D* AdvancedHitTexture = TextureWeakPtr.Get();
	FTexturePlatformData* PlatformData = AdvancedHitTexture ? AdvancedHitTexture->GetPlatformData() : nullptr;
	if (!PlatformData)
	{
		return bSucceed;
	}

	FBulkDataInterface* BulkDataPtr = &PlatformData->Mips[0].BulkData;
	const void* RawImage = BulkDataPtr->Lock(LOCK_READ_ONLY);
	if (!ensureMsgf(RawImage, TEXT("ASSERT: 'RawImage' is not valid, could not lock the bulk data")))
	{
		BulkDataPtr->Unlock();
		return bSucceed;
	}

	const FColor* RawColorArray = static_cast<const FColor*>(RawImage);
	if (!ensureMsgf(RawColorArray, TEXT("ASSERT: 'RawColorArray' is not valid")))
	{
		BulkDataPtr->Unlock();
		return bSucceed;
	}

	FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	LocalPosition.X = floor(LocalPosition.X);
	LocalPosition.Y = floor(LocalPosition.Y);
	LocalPosition /= MyGeometry.GetLocalSize();

	const int32 ImageWidth = PlatformData->SizeX;
	LocalPosition.X *= ImageWidth;
	LocalPosition.Y *= PlatformData->SizeY;

	const int32 BufferPosition = floor(LocalPosition.Y) * ImageWidth + LocalPosition.X;

	// Return false if alpha pixel hovered
	if (RawColorArray[BufferPosition].A <= AdvancedHitAlpha)
	{
		bSucceed = false;
	}

	BulkDataPtr->Unlock();
	return bSucceed;
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
