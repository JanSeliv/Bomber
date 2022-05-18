// Copyright (c) Yevhenii Selivanov.

#include "UI/ShapeButton.h"
//---
#include "UMG.h"
#include "RenderingThread.h"

SShapeButton::~SShapeButton()
{
	RawColorsPtr.Reset();
}

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

/** Allows button to be hovered. */
void SShapeButton::SetCanHover(bool bAllow)
{
	if (bCanHover == bAllow)
	{
		return;
	}

	bCanHover = bAllow;

	TAttribute<bool> bHovered = false;
	if (bAllow)
	{
		bHovered.Bind(this, &SShapeButton::IsAlphaPixelHovered);
	}

	SetHover(bHovered);
	ExecuteHoverStateChanged(true);
}

void SShapeButton::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SButton::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	TickDetectMouseLeave();
}

void SShapeButton::Release()
{
	SButton::Release();

	SetCanHover(false);
}

FReply SShapeButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!IsAlphaPixelHovered())
	{
		// Avoid press event
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SShapeButton::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsAlphaPixelHovered())
	{
		// Avoid press event
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SShapeButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!IsAlphaPixelHovered())
	{
		// Avoid press event
		return FReply::Unhandled();
	}
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SShapeButton::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	CurrentGeometry = MyGeometry;
	CurrentMouseEvent = MouseEvent;

	if (!bCanHover
	    && IsAlphaPixelHovered())
	{
		// Allow to be hovered since mouse is hovered on alpha pixel
		SetCanHover(true);
	}

	return SButton::OnMouseMove(MyGeometry, MouseEvent);
}

void SShapeButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SButton::OnMouseLeave(MouseEvent);

	SetCanHover(false);
}

void SShapeButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SButton::OnMouseEnter(MyGeometry, MouseEvent);

	TryUpdateRawColorsOnce();

	SetCanHover(true);
}

// Returns true if cursor is hovered on a texture
bool SShapeButton::IsAlphaPixelHovered() const
{
	const bool bIsCurrentlyHovered = IsHovered();

	const FVector2D CurrentGeometrySize = CurrentGeometry.GetLocalSize();
	if (CurrentGeometrySize.IsZero()
	    || !AdvancedHitAlpha)
	{
		return bIsCurrentlyHovered;
	}

	const TArray<FColor>* RawColors = RawColorsPtr.Get();
	if (!RawColors
	    || !RawColors->Num())
	{
		// Raw Colors are not set
		return bIsCurrentlyHovered;
	}

	FVector2D LocalPosition = CurrentGeometry.AbsoluteToLocal(CurrentMouseEvent.GetScreenSpacePosition());
	LocalPosition.X = FMath::Floor(LocalPosition.X);
	LocalPosition.Y = FMath::Floor(LocalPosition.Y);
	LocalPosition /= CurrentGeometrySize;
	LocalPosition.X *= TextureRes.X;
	LocalPosition.Y *= TextureRes.Y;
	const int32 BufferPosition = FMath::Floor(LocalPosition.Y) * TextureRes.X + LocalPosition.X;

	const TArray<FColor>& RawColorsArray = *RawColors;
	if (!RawColorsArray.IsValidIndex(BufferPosition))
	{
		return bIsCurrentlyHovered;
	}

	const bool bIsAlphaPixelHovered = RawColorsArray[BufferPosition].A > AdvancedHitAlpha;
	return bIsAlphaPixelHovered;
}

// Set once on render thread the buffer data about all pixels of current texture if was not set before
void SShapeButton::TryUpdateRawColorsOnce()
{
	if (RawColorsPtr)
	{
		// Is already valid
		return;
	}

	const UTexture2D* HitTexture = TextureWeakPtr.Get();
	if (!HitTexture)
	{
		return;
	}

	TextureRes = FIntPoint(HitTexture->GetSizeX(), HitTexture->GetSizeY());

	// Create
	RawColorsPtr = MakeShared<TArray<FColor>, ESPMode::ThreadSafe>();
	RawColorsPtr->SetNum(TextureRes.X * TextureRes.Y);

	// Get Raw Colors data on Render thread
	const TWeakPtr<TArray<FColor>, ESPMode::ThreadSafe> RawColorsWeakPtr = RawColorsPtr;
	ENQUEUE_RENDER_COMMAND(TryUpdateRawColorsOnce)([RawColorsWeakPtr, WeakTexture = TextureWeakPtr](FRHICommandListImmediate&)
	{
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
		TArray<FColor>* RawColors = RawColorsWeakPtr.Pin().Get();
		check(RawColors);
		const int32 Count = RawColors->Num() * sizeof(FColor);
		FMemory::Memcpy(/*dest*/RawColors->GetData(), /*source*/MappedTextureMemory, Count);

		// Unlock
		RHIUnlockTexture2D(RHITexture2D, MipIndex, bLockWithinMipTail);
	});
}

// Try register leaving the button (e.g. another widget opens above)
void SShapeButton::TickDetectMouseLeave()
{
	if (bCanHover
	    && IsHovered()
	    && CurrentGeometry.GetLocalSize().IsZero())
	{
		// Current data is zero, so widget is not hovered anymore
		OnMouseLeave(CurrentMouseEvent);
	}

	// Reset data every tick, it will actualised during OnMouseMove
	// so if data is empty, then Mouse Move did not happen
	// and widget is not hovered anymore
	static const FGeometry EmptyGeometry{};
	CurrentGeometry = EmptyGeometry;
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
	SetClickMethod(EButtonClickMethod::PreciseClick);
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
