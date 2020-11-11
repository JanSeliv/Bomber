// Copyright 2020 Yevhenii Selivanov.

#include "UI/ShapeButton.h"
//---
#include "UMG.h"

//
void SShapeButton::SetAdvancedHitTexture(UTexture2D* InTexture)
{
	AdvancedHitTexture = InTexture;
}

//
void SShapeButton::SetAdvancedHitAlpha(int32 InAlpha)
{
	AdvancedHitAlpha = InAlpha;
}

//
FReply SShapeButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!NeedExecuteAction(MyGeometry, MouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
}

//
FReply SShapeButton::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (!NeedExecuteAction(InMyGeometry, InMouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

//
FReply SShapeButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!NeedExecuteAction(MyGeometry, MouseEvent))
	{
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}

//
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

//
void SShapeButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (AdvancedHitTexture)
	{
		return;
	}
	return SButton::OnMouseEnter(MyGeometry, MouseEvent);
}

//
void SShapeButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	return SButton::OnMouseLeave(MouseEvent);
}

//
FCursorReply SShapeButton::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (!bIsHovered)
	{
		return FCursorReply::Unhandled();
	}
	TOptional<EMouseCursor::Type> TheCursor = Cursor.Get();
	return TheCursor.IsSet() ? FCursorReply::Cursor(TheCursor.GetValue()) : FCursorReply::Unhandled();
}

//
TSharedPtr<IToolTip> SShapeButton::GetToolTip()
{
	return bIsHovered ? SWidget::GetToolTip() : nullptr;
}

//
bool SShapeButton::NeedExecuteAction(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	FTexturePlatformData* const& PlatformData = AdvancedHitTexture ? AdvancedHitTexture->PlatformData : nullptr;
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
	FTexture2DMipMap& Mip = PlatformData->Mips[0];
	void* const& ImageData = Mip.BulkData.Lock(LOCK_READ_ONLY);
	if (!ImageData)
	{
		bToReturn = false;
	}
	else
	{
		FColor* const& ColorData = static_cast<FColor*>(ImageData);
		if (!ColorData
		    || ColorData[BufferPosition].A <= AdvancedHitAlpha)
		{
			bToReturn = false;
		}
	}
	Mip.BulkData.Unlock();

	return bToReturn;
}

//
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

//
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

//
void UShapeButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	SetAdvancedHitTexture(AdvancedHitTexture);
	SetAdvancedHitAlpha(AdvancedHitAlpha);
}

//
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
