// Copyright (c) Yevhenii Selivanov.

#include "CustomShapeButton.h"
//---
#include "SCustomShapeButton.h"
//---
#include "Components/ButtonSlot.h"
#include "Engine/Texture2D.h"
#include "Framework/SlateDelegates.h"

// Set texture to collide with specified texture
void UCustomShapeButton::SetAdvancedHitTexture(UTexture2D* InTexture)
{
	SButton* MyButtonPtr = MyButton.Get();
	if (!MyButtonPtr
		|| !InTexture)
	{
		return;
	}

	AdvancedHitTexture = InTexture;
	if (SCustomShapeButton* ShapeButton = static_cast<SCustomShapeButton*>(MyButtonPtr))
	{
		ShapeButton->SetAdvancedHitTexture(AdvancedHitTexture);
	}
}

// Set new alpha
void UCustomShapeButton::SetAdvancedHitAlpha(int32 InAlpha)
{
	SButton* MyButtonPtr = MyButton.Get();
	if (!MyButtonPtr)
	{
		return;
	}

	AdvancedHitAlpha = InAlpha;
	if (SCustomShapeButton* ShapeButton = static_cast<SCustomShapeButton*>(MyButtonPtr))
	{
		ShapeButton->SetAdvancedHitAlpha(AdvancedHitAlpha);
	}
}

// Applies all properties to the native widget if possible
void UCustomShapeButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	SetAdvancedHitTexture(AdvancedHitTexture);
	SetAdvancedHitAlpha(AdvancedHitAlpha);
	SetClickMethod(EButtonClickMethod::PreciseClick);
}

// Is called when the underlying SWidget needs to be constructed
TSharedRef<SWidget> UCustomShapeButton::RebuildWidget()
{
	const TSharedRef<SCustomShapeButton> NewButtonRef = SNew(SCustomShapeButton)
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
