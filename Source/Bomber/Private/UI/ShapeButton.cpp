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
	const bool WhatToReturn = NeedExecuteAction(MyGeometry, MouseEvent);
	if (WhatToReturn != bIsHovered)
	{
		bIsHovered = WhatToReturn;
		if (bIsHovered)
		{
			SButton::OnMouseEnter(MyGeometry, MouseEvent);
		}
		else
		{
			SButton::OnMouseLeave(MouseEvent);
		}
	}
	return SButton::OnMouseMove(MyGeometry, MouseEvent);
}

void SShapeButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (TextureWeakPtr.IsValid())
	{
		return SButton::OnMouseEnter(MyGeometry, MouseEvent);
	}
}

void SShapeButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	return SButton::OnMouseLeave(MouseEvent);
}

FCursorReply SShapeButton::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (!bIsHovered)
	{
		return FCursorReply::Unhandled();
	}
	TOptional<EMouseCursor::Type> TheCursor = Cursor.Get();
	return TheCursor.IsSet() ? FCursorReply::Cursor(TheCursor.GetValue()) : FCursorReply::Unhandled();
}

TSharedPtr<IToolTip> SShapeButton::GetToolTip()
{
	return bIsHovered ? SWidget::GetToolTip() : nullptr;
}

// Called on mouse moves and clicks
bool SShapeButton::NeedExecuteAction(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	const UTexture2D* AdvancedHitTexture = TextureWeakPtr.Get();
	FTexturePlatformData* PlatformData = AdvancedHitTexture ? AdvancedHitTexture->PlatformData : nullptr;
	if (!PlatformData)
	{
		return true;
	}

	FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	LocalPosition.X = floor(LocalPosition.X);
	LocalPosition.Y = floor(LocalPosition.Y);
	LocalPosition /= MyGeometry.GetLocalSize();

	const int32 ImageWidth = PlatformData->SizeX;
	LocalPosition.X *= ImageWidth;
	LocalPosition.Y *= PlatformData->SizeY;

	const int32 BufferPosition = (floor(LocalPosition.Y) * ImageWidth) + LocalPosition.X;

	if (!PlatformData->Mips.IsValidIndex(0))
	{
		return true;
	}

	bool bToReturn = true;
	FByteBulkData& BulkDataRef = PlatformData->Mips[0].BulkData;
	const void* ImageData = BulkDataRef.Lock(LOCK_READ_ONLY);
	if (!ImageData)
	{
		bToReturn = false;
	}
	else
	{
		const auto ColorPtr = static_cast<const FColor*>(ImageData);
		if (!ColorPtr
		    || ColorPtr[BufferPosition].A <= AdvancedHitAlpha)
		{
			bToReturn = false;
		}
	}
	BulkDataRef.Unlock();

	return bToReturn;
}

// Set texture to collide with specified texture
void UShapeButton::SetAdvancedHitTexture(UTexture2D* InTexture)
{
	AdvancedHitTexture = InTexture;
	if (!MyButton.IsValid())
	{
		return;
	}

	if (const auto& ShapeButton = static_cast<SShapeButton*>(MyButton.Get()))
	{
		ShapeButton->SetAdvancedHitTexture(AdvancedHitTexture);
	}
}

// Set new alpha
void UShapeButton::SetAdvancedHitAlpha(int32 InAlpha)
{
	AdvancedHitAlpha = InAlpha;
	if (!MyButton.IsValid())
	{
		return;
	}

	if (const auto& ShapeButton = static_cast<SShapeButton*>(MyButton.Get()))
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
	TSharedPtr<SShapeButton> NewButton = SNew(SShapeButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.OnHovered_UObject(this, &ThisClass::SlateHandleHovered)
		.OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhovered)
		.ButtonStyle(&WidgetStyle)
		.ClickMethod(ClickMethod)
		.TouchMethod(TouchMethod)
		.IsFocusable(IsFocusable);

	NewButton->SetAdvancedHitTexture(AdvancedHitTexture);
	NewButton->SetAdvancedHitAlpha(AdvancedHitAlpha);

	MyButton = NewButton;

	if (GetChildrenCount())
	{
		if (auto ButtonSlot = Cast<UButtonSlot>(GetContentSlot()))
		{
			ButtonSlot->BuildSlot(MyButton.ToSharedRef());
		}
	}

	return MyButton.ToSharedRef();
}
