// Copyright (c) Yevhenii Selivanov

#include "SCustomShapeButton.h"
//---
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"

// Virtual destructor, unregister data
SCustomShapeButton::~SCustomShapeButton()
{
	RawColorsPtr.Reset();
}

/** Allows button to be hovered. */
void SCustomShapeButton::SetCanHover(bool bAllow)
{
	if (bCanHover == bAllow)
	{
		return;
	}

	bCanHover = bAllow;

	TAttribute<bool> bHovered = false;
	if (bAllow)
	{
		bHovered.Bind(this, &SCustomShapeButton::IsAlphaPixelHovered);
	}

	SetHover(bHovered);

	TryDetectOnHovered();
}

void SCustomShapeButton::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SButton::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	TickDetectMouseLeave(InDeltaTime);
}

FReply SCustomShapeButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UpdateMouseData(MyGeometry, MouseEvent);

	if (!IsHovered())
	{
		// Avoid press event
		return FReply::Unhandled();
	}

	return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SCustomShapeButton::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UpdateMouseData(MyGeometry, MouseEvent);

	if (!IsHovered())
	{
		// Avoid press event
		return FReply::Unhandled();
	}

	return SButton::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
}

FReply SCustomShapeButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UpdateMouseData(MyGeometry, MouseEvent);

	FReply Reply = FReply::Unhandled();
	if (IsHovered())
	{
		Reply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
	}

	SetCanHover(false);

	return Reply;
}

FReply SCustomShapeButton::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UpdateMouseData(MyGeometry, MouseEvent);

	if (!bCanHover
		&& IsAlphaPixelHovered())
	{
		// Allow to be hovered since mouse is hovered on alpha pixel
		SetCanHover(true);
	}

	TryDetectOnHovered();

	return SButton::OnMouseMove(MyGeometry, MouseEvent);
}

void SCustomShapeButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SButton::OnMouseLeave(MouseEvent);

	if (!IsAlphaPixelHovered())
	{
		SetCanHover(false);
	}
}

void SCustomShapeButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SButton::OnMouseEnter(MyGeometry, MouseEvent);

	TryUpdateRawColorsOnce();

	SetCanHover(true);
}

// Returns true if cursor is hovered on a texture
bool SCustomShapeButton::IsAlphaPixelHovered() const
{
	const FVector2D CurrentGeometrySize = CurrentGeometry.GetLocalSize();
	if (CurrentGeometrySize.IsZero())
	{
		return false;
	}

	const TArray<FColor>* RawColors = RawColorsPtr.Get();
	if (!RawColors
		|| !RawColors->Num())
	{
		// Raw Colors are not set
		return false;
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
		return false;
	}

	constexpr int32 Alpha = 1;
	const bool bIsAlphaPixelHovered = RawColorsArray[BufferPosition].A > Alpha;
	return bIsAlphaPixelHovered;
}

// Set once on render thread the buffer data about all pixels of current texture if was not set before
void SCustomShapeButton::TryUpdateRawColorsOnce()
{
	if (RawColorsPtr)
	{
		// Is already valid
		return;
	}

	const FSlateBrush* ImageBrush = GetBorderImage();
	const UTexture2D* ButtonTexture = ImageBrush ? Cast<UTexture2D>(ImageBrush->GetResourceObject()) : nullptr;
	if (!ensureMsgf(ButtonTexture, TEXT("%s: 'HitTexture' is null, most likely no texture is set in the Button Style"), *FString(__FUNCTION__)))
	{
		return;
	}

	TextureRes = FIntPoint(ButtonTexture->GetSizeX(), ButtonTexture->GetSizeY());

	// Create
	RawColorsPtr = MakeShared<TArray<FColor>, ESPMode::ThreadSafe>();
	RawColorsPtr->SetNum(TextureRes.X * TextureRes.Y);

	// Get Raw Colors data on Render thread
	const TWeakPtr<TArray<FColor>, ESPMode::ThreadSafe> InOutRawColorsWeakPtr = RawColorsPtr;
	const TWeakObjectPtr<const UTexture2D> WeakTexture = ButtonTexture;
	ENQUEUE_RENDER_COMMAND(TryUpdateRawColorsOnce)([InOutRawColorsWeakPtr, WeakTexture](FRHICommandListImmediate&)
	{
		TArray<FColor>* RawColors = InOutRawColorsWeakPtr.Pin().Get();
		if (!ensureMsgf(RawColors, TEXT("%s: 'RawColors' is null, can not obtain its data"), *FString(__FUNCTION__)))
		{
			return;
		}

		const UTexture2D* Texture2D = WeakTexture.Get();
		const FTextureResource* TextureResource = Texture2D ? Texture2D->GetResource() : nullptr;
		FRHITexture2D* RHITexture2D = TextureResource ? TextureResource->GetTexture2DRHI() : nullptr;
		if (!ensureMsgf(RHITexture2D, TEXT("%s: 'RHITexture2D' is not valid"), *FString(__FUNCTION__)))
		{
			return;
		}

		// Lock
		uint32 DestPitch = 0;
		constexpr int32 MipIndex = 0;
		constexpr bool bLockWithinMipTail = false;
		const uint8* MappedTextureMemory = static_cast<const uint8*>(RHILockTexture2D(RHITexture2D, MipIndex, RLM_ReadOnly, DestPitch, bLockWithinMipTail));

		// Copy data
		const int32 Count = RawColors->Num() * sizeof(FColor);
		FMemory::Memcpy(/*dest*/RawColors->GetData(), /*source*/MappedTextureMemory, Count);

		// Unlock
		RHIUnlockTexture2D(RHITexture2D, MipIndex, bLockWithinMipTail);
	});
}

// Try register leaving the button (e.g. another widget opens above)
void SCustomShapeButton::TickDetectMouseLeave(float DeltaTime)
{
	if (bCanHover
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

// Try register On Hovered and On Unhovered events
void SCustomShapeButton::TryDetectOnHovered()
{
	const bool bIsHoveredNow = bCanHover && IsHovered();
	if (bIsHovered != bIsHoveredNow)
	{
		// Call OnHovered\OnUnhovered event
		ExecuteHoverStateChanged(true);
	}

	bIsHovered = bIsHoveredNow;
}

// Caching current geometry and last mouse event
void SCustomShapeButton::UpdateMouseData(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	CurrentGeometry = MyGeometry;
	CurrentMouseEvent = MouseEvent;
}
