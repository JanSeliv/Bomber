// Copyright (c) Yevhenii Selivanov

#include "UI/GfpmSwitcherRowWidget.h"

// GFPM
#include "GfpmUtils.h"

// UE
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"

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

// When Package button is pressed
void UGfpmSwitcherRowWidget::OnPackageButtonPressed_Implementation()
{
	OnPackageRequested.Broadcast(GameFeatureName);
}

// When this widget is constructed
void UGfpmSwitcherRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ActiveCheckBox)
	{
		ActiveCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnActiveCheckStateChanged);
	}

	if (PackageButton)
	{
		// Packaging plugin as mod is developer action, hide button outside editor-not-PIE world
		const UWorld* World = GetWorld();
		const bool bIsEditorNotPie = World && World->WorldType == EWorldType::Editor;
		PackageButton->SetVisibility(bIsEditorNotPie ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

#if WITH_EDITOR
		PackageButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnPackageButtonPressed);

		// Set tooltip from reflection property comment
		if (const FProperty* PackageButtonProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UGfpmSwitcherRowWidget, PackageButton)))
		{
			PackageButton->SetToolTipText(PackageButtonProperty->GetToolTipText());
		}
#endif // WITH_EDITOR
	}
}

// When this widget is destroyed
void UGfpmSwitcherRowWidget::NativeDestruct()
{
	if (ActiveCheckBox)
	{
		ActiveCheckBox->OnCheckStateChanged.RemoveAll(this);
	}

#if WITH_EDITOR
	if (PackageButton)
	{
		PackageButton->OnClicked.RemoveAll(this);
	}
#endif // WITH_EDITOR

	Super::NativeDestruct();
}
