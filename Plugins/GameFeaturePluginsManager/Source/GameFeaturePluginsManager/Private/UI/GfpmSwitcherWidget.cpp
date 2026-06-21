// Copyright (c) Yevhenii Selivanov

#include "UI/GfpmSwitcherWidget.h"

// GFPM
#include "GfpmUtils.h"
#include "UI/GfpmSwitcherRowWidget.h"

// UE
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Engine/World.h"
#include "GameFeaturesSubsystem.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmSwitcherWidget)

#if !UE_BUILD_SHIPPING
// Debug console command to flip-flop the switcher on the viewport without the settings menu, stripped from shipping
static FAutoConsoleCommandWithWorld GToggleGameFeaturesSwitcherCommand(
    TEXT("ToggleGameFeaturesSwitcher"),
    TEXT("Toggle the Game Feature Plugins switcher widget on the viewport"),
    FConsoleCommandWithWorldDelegate::CreateLambda([](const UWorld* World)
{
	if (UGfpmSwitcherWidget* SwitcherWidget = UGfpmSwitcherWidget::GetRuntimeSwitcherWidget(World))
	{
		SwitcherWidget->ToggleSwitcher();
	}
}));
#endif // !UE_BUILD_SHIPPING

// Returns switcher added to viewport of given context's play world, nullptr if none
UGfpmSwitcherWidget* UGfpmSwitcherWidget::GetRuntimeSwitcherWidget(const UObject* WorldContext)
{
	TArray<UUserWidget*> FoundWidgets;
	constexpr bool bTopLevelOnly = false;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(WorldContext, FoundWidgets, StaticClass(), bTopLevelOnly);
	return FoundWidgets.IsEmpty() ? nullptr : Cast<UGfpmSwitcherWidget>(FoundWidgets[0]);
}

// When Mods settings button is pressed
void UGfpmSwitcherWidget::OnSwitcherOpened_Implementation()
{
	// Rebuild in case the registered plugin set changed since construction
	RebuildRows();
	SetVisibility(ESlateVisibility::Visible);
}

// Collapses switcher back, bound to in-widget Close button
void UGfpmSwitcherWidget::CloseSwitcher()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

// Flip-flops switcher visibility, used by debug cheats to drive it without settings menu
void UGfpmSwitcherWidget::ToggleSwitcher()
{
	if (IsVisible())
	{
		CloseSwitcher();
	}
	else
	{
		OnSwitcherOpened();
	}
}

// Presents switcher as always-open embedded panel where host (editor tab) owns closing
void UGfpmSwitcherWidget::ShowInEditorTab()
{
	OnSwitcherOpened();

	if (CloseButton)
	{
		// Host tab provides its own closing affordance, so in-widget Close button is redundant here
		CloseButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// Rebuilds rows from all registered Game Feature Plugins
void UGfpmSwitcherWidget::RebuildRows()
{
	if (!PluginsList
	    || !RowWidgetClass)
	{
		// Widgets not yet bound
		return;
	}

	PluginsList->ClearChildren();

	const TArray<FString> RegisteredPlugins = UGfpmUtils::GetAllRegisteredGameFeaturePlugins();
	for (const FString& PluginName : RegisteredPlugins)
	{
		UGfpmSwitcherRowWidget* Row = CreateWidget<UGfpmSwitcherRowWidget>(this, RowWidgetClass);
		if (!ensureMsgf(Row, TEXT("ASSERT: [%i] %hs:\n'Row' condition is FALSE"), __LINE__, __FUNCTION__))
		{
			// Designer row class failed to construct, skip this entry
			continue;
		}

		Row->InitRow(FName(*PluginName));
		Row->OnPackageRequested.AddWeakLambda(this, [this](FName GameFeatureName)
		{
			OnPackageGameFeatureRequested.Broadcast(GameFeatureName);
		});
		PluginsList->AddChild(Row);
	}
}

// Tells every row to re-read its plugin's active state
void UGfpmSwitcherWidget::RefreshRowStates()
{
	if (!PluginsList)
	{
		// Widget not yet bound
		return;
	}

	const int32 ChildrenCount = PluginsList->GetChildrenCount();
	for (int32 Index = 0; Index < ChildrenCount; ++Index)
	{
		if (UGfpmSwitcherRowWidget* Row = Cast<UGfpmSwitcherRowWidget>(PluginsList->GetChildAt(Index)))
		{
			Row->RefreshActiveState();
		}
	}
}

// When Close button is pressed
void UGfpmSwitcherWidget::OnCloseButtonPressed_Implementation()
{
	CloseSwitcher();
}

// When Package All button is pressed
void UGfpmSwitcherWidget::OnPackageAllButtonPressed_Implementation()
{
	OnPackageAllRequested.Broadcast();
}

// When widget is constructed and added to viewport
void UGfpmSwitcherWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCloseButtonPressed);
	}

	if (PackageAllButton)
	{
		// Packaging is developer action, hide Package All outside editor-not-PIE world
		const UWorld* World = GetWorld();
		const bool bIsEditorNotPie = World && World->WorldType == EWorldType::Editor;
		PackageAllButton->SetVisibility(bIsEditorNotPie ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

#if WITH_EDITOR
		PackageAllButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnPackageAllButtonPressed);
#endif // WITH_EDITOR
	}

	RebuildRows();

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);

	// Start hidden everywhere, runtime opens it via Mods setting, editor host opens it via ShowInEditorTab
	SetVisibility(ESlateVisibility::Collapsed);
}

// When widget is destroyed and removed from viewport
void UGfpmSwitcherWidget::NativeDestruct()
{
	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
	}

#if WITH_EDITOR
	if (PackageAllButton)
	{
		PackageAllButton->OnClicked.RemoveAll(this);
	}
#endif // WITH_EDITOR

	Super::NativeDestruct();
}

// When owning game feature plugin starts activating
void UGfpmSwitcherWidget::OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	RefreshRowStates();
}

// When owning game feature plugin starts deactivating
void UGfpmSwitcherWidget::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& ContextRef, const FString& PluginURL)
{
	RefreshRowStates();
}
