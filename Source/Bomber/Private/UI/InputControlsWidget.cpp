// Copyright (c) Yevhenii Selivanov

#include "UI/InputControlsWidget.h"
//---
#include "Components/InputKeySelector.h"
#include "Widgets/Input/SInputKeySelector.h"

// Sets the style of the button and its text
void UInputControlsWidget::SetStyle(UInputKeySelector* InputKeySelector, const FTextBlockStyle& TextStyle, const FButtonStyle& ButtonStyle)
{
	SInputKeySelector* SlateInputKeySelector = GetSlateWidget<SInputKeySelector>(InputKeySelector).Get();
	if (!ensureMsgf(SlateInputKeySelector, TEXT("ASSERT: 'SlateInputKeySelector' is not valid")))
	{
		return;
	}

	SlateInputKeySelector->SetTextStyle(&TextStyle);
	InputKeySelector->TextStyle = TextStyle;

	SlateInputKeySelector->SetButtonStyle(&ButtonStyle);
	InputKeySelector->WidgetStyle = ButtonStyle;
}

// Called after the underlying slate widget is constructed.
void UInputControlsWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

// Updates appearance dynamically in the editor
void UInputControlsWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}
