// Copyright 2021 Yevhenii Selivanov

#include "UI/InputControlsWidget.h"
//---
#include "Globals/SingletonLibrary.h"
#include "UI/MyHUD.h"

// Display the Input Controls widget on UI
void UInputControlsWidget::OpenWidget()
{
	SetVisibility(ESlateVisibility::Visible);
}

// Close the Input Controls widget
void UInputControlsWidget::CloseWidget()
{
	if (!IsVisible())
	{
		// Widget is already closed
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

// Called after the underlying slate widget is constructed.
void UInputControlsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	OnVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnVisibilityChange);
}

// Updates appearance dynamically in the editor
void UInputControlsWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

// Is called when visibility is changed for this widget
void UInputControlsWidget::OnVisibilityChange(ESlateVisibility InVisibility)
{
	AMyHUD* MyHUD = USingletonLibrary::GetMyHUD();
	if (!MyHUD)
	{
		return;
	}

	if (InVisibility == ESlateVisibility::Visible)
	{
		MyHUD->OnClose.AddUniqueDynamic(this, &ThisClass::CloseWidget);
	}
	else if (MyHUD->OnClose.IsAlreadyBound(this, &ThisClass::CloseWidget))
	{
		MyHUD->OnClose.RemoveDynamic(this, &ThisClass::CloseWidget);
	}
}
