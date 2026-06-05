// Copyright (c) Yevhenii Selivanov

#include "UI/GfpmSwitcherRowWidget.h"

// GFPM
#include "GfpmUtils.h"

// UE
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmSwitcherRowWidget)

// Binds this row to Game Feature Plugin and shows its current active state
void UGfpmSwitcherRowWidget::InitRow(FName InGameFeatureName)
{
	GameFeatureName = InGameFeatureName;

	if (PluginNameText)
	{
		PluginNameText->SetText(FText::FromName(InGameFeatureName));
	}

	RefreshActiveState();
}

// Re-reads plugin active state into toggle, used when another switcher changed it
void UGfpmSwitcherRowWidget::RefreshActiveState()
{
	if (!ActiveCheckBox)
	{
		// Widget not yet constructed
		return;
	}

	constexpr bool bCheckForPending = true;
	const bool bIsActive = UGfpmUtils::IsGameFeaturePluginActive(GameFeatureName, bCheckForPending);
	if (ActiveCheckBox->IsChecked() != bIsActive)
	{
		ActiveCheckBox->SetIsChecked(bIsActive);
	}
}

// When active check state changes
void UGfpmSwitcherRowWidget::OnActiveCheckStateChanged_Implementation(bool bIsChecked)
{
	UGfpmUtils::SetGameFeaturePluginsActive(bIsChecked, {GameFeatureName});
}

// When this widget is constructed
void UGfpmSwitcherRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ActiveCheckBox)
	{
		ActiveCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnActiveCheckStateChanged);
	}
}

// When this widget is destructed
void UGfpmSwitcherRowWidget::NativeDestruct()
{
	if (ActiveCheckBox)
	{
		ActiveCheckBox->OnCheckStateChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}
