// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GfpmSwitcherEditorSubsystem.h"

// GFPM
#include "UI/GfpmSwitcherWidget.h"

// UE
#include "Blueprint/UserWidget.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/LayoutExtender.h"
#include "Framework/Docking/TabManager.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SNullWidget.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmSwitcherEditorSubsystem)

namespace GfpmSwitcherTab
{
	/** Tab id of editor switcher docked panel. */
	static const FName TabName(TEXT("GfpmSwitcher"));

	/** Level-editor tab switcher docks next to by default. */
	static const FName OutlinerTabName(TEXT("LevelEditorSceneOutliner"));
} // namespace GfpmSwitcherTab

// Returns level-editor tab manager that owns docked switcher tab, nullptr when unavailable
TSharedPtr<FTabManager> UGfpmSwitcherEditorSubsystem::GetSwitcherTabManager() const
{
	const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
	return LevelEditorModule ? LevelEditorModule->GetLevelEditorTabManager() : nullptr;
}

// Finds live switcher tab in level-editor tab manager, nullptr when not open
TSharedPtr<SDockTab> UGfpmSwitcherEditorSubsystem::FindSwitcherTab() const
{
	const TSharedPtr<FTabManager> LevelEditorTabManager = GetSwitcherTabManager();
	return LevelEditorTabManager.IsValid() ? LevelEditorTabManager->FindExistingLiveTab(FTabId(GfpmSwitcherTab::TabName)) : nullptr;
}

// Opens or focuses switcher tab in its docked spot, also action behind Window menu entry
void UGfpmSwitcherEditorSubsystem::OpenSwitcherTab()
{
	// Invoke through level-editor tab manager that owns spawner, so it lands in docked spot
	const TSharedPtr<FTabManager> LevelEditorTabManager = GetSwitcherTabManager();
	if (LevelEditorTabManager.IsValid())
	{
		LevelEditorTabManager->TryInvokeTab(FTabId(GfpmSwitcherTab::TabName));
	}
}

// Closes switcher tab if currently open
void UGfpmSwitcherEditorSubsystem::CloseSwitcherTab()
{
	const TSharedPtr<SDockTab> ExistingTab = FindSwitcherTab();
	if (ExistingTab.IsValid())
	{
		ExistingTab->RequestCloseTab();
	}
}

// Returns true if switcher tab is currently open in level-editor tab manager
bool UGfpmSwitcherEditorSubsystem::IsSwitcherTabOpen() const
{
	return FindSwitcherTab().IsValid();
}

// When tab manager needs to spawn switcher dockable tab
TSharedRef<SDockTab> UGfpmSwitcherEditorSubsystem::OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTab> DockTab = SNew(SDockTab);
	DockTab->SetContent(MakeSwitcherTabContent());

	// Drop kept-alive widget once its tab goes away so reopen rebuilds fresh one
	DockTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateWeakLambda(this, [this](TSharedRef<SDockTab>)
	{
		SwitcherWidget = nullptr;
	}));

	return DockTab;
}

// Creates switcher widget on editor world and returns its Slate content for tab
TSharedRef<SWidget> UGfpmSwitcherEditorSubsystem::MakeSwitcherTabContent()
{
	UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!ensureMsgf(EditorWorld, TEXT("ASSERT: [%i] %hs:\n'EditorWorld' is not available!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!SwitcherWidgetClass.IsNull(), TEXT("ASSERT: [%i] %hs:\n'SwitcherWidgetClass' is not set!"), __LINE__, __FUNCTION__))
	{
		return SNullWidget::NullWidget;
	}

	// Editor-only subsystem, which is never running in build, so blocking load widget for editor tab
	SwitcherWidget = CreateWidget<UUserWidget>(EditorWorld, SwitcherWidgetClass.LoadSynchronous());
	checkf(SwitcherWidget, TEXT("ERROR: [%i] %hs:\nFailed to create switcher widget from class: %s!"), __LINE__, __FUNCTION__, *SwitcherWidgetClass.ToString());

	const TSharedRef<SWidget> TabContent = SwitcherWidget->TakeWidget();
	if (UGfpmSwitcherWidget* Switcher = Cast<UGfpmSwitcherWidget>(SwitcherWidget))
	{
		// Present as always-open embedded panel, tab owns closing
		Switcher->ShowInEditorTab();
	}
	return TabContent;
}

// When Blueprint asset is reinstanced in editor
void UGfpmSwitcherEditorSubsystem::OnBlueprintReinstanced_Implementation()
{
	const TSharedPtr<SDockTab> SwitcherTab = FindSwitcherTab();
	if (SwitcherTab.IsValid())
	{
		// Reinstancing tears down hosted widget Slate, recreate from freshly compiled class
		SwitcherTab->SetContent(MakeSwitcherTabContent());
	}
}

// Registers switcher tab spawner on level-editor tab manager, no-op if already registered or unavailable
void UGfpmSwitcherEditorSubsystem::RegisterTabSpawner(TSharedPtr<FTabManager> InTabManager)
{
	if (!InTabManager.IsValid()
	    || InTabManager->HasTabSpawner(GfpmSwitcherTab::TabName))
	{
		// No tab manager yet, or this one already owns spawner, e.g. layout was rebuilt
		return;
	}

	const IWorkspaceMenuStructure& MenuStructure = WorkspaceMenu::GetMenuStructure();
	InTabManager->RegisterTabSpawner(GfpmSwitcherTab::TabName, FOnSpawnTab::CreateUObject(this, &ThisClass::OnSpawnTab))
	    .SetDisplayName(FText::FromString(TEXT("Game Features")))
	    .SetTooltipText(FText::FromString(TEXT("Open the Game Feature Plugins Manager panel")))
	    .SetGroup(MenuStructure.GetLevelEditorCategory());
}

// When subsystem initializes
void UGfpmSwitcherEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!FSlateApplication::IsInitialized())
	{
		// Headless contexts (cook, commandlets) have no Slate, so dockable tab cannot exist
		return;
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	// Register spawner on level-editor tab manager when layout is (re)built, so it docks like built-in panels
	RegisterTabsHandle = LevelEditorModule.OnRegisterTabs().AddUObject(this, &ThisClass::RegisterTabSpawner);

	// Dock switcher next to Scene Outliner and open by default, so it appears where panels normally live
	LayoutExtensionHandle = LevelEditorModule.OnRegisterLayoutExtensions().AddWeakLambda(this, [](FLayoutExtender& Extender)
	{
		Extender.ExtendLayout(FTabId(GfpmSwitcherTab::OutlinerTabName), ELayoutExtensionPosition::After, FTabManager::FTab(FTabId(GfpmSwitcherTab::TabName), ETabState::OpenedTab));
	});

	// Recompiling switcher Widget Blueprint reinstances its hosted widget, refresh open tab so it does not go blank
	if (GEditor)
	{
		BlueprintReinstancedHandle = GEditor->OnBlueprintReinstanced().AddUObject(this, &ThisClass::OnBlueprintReinstanced);
	}

	// Cover late initialization, register now if layout was already built so Window menu entry works this session
	RegisterTabSpawner(LevelEditorModule.GetLevelEditorTabManager());
}

// When subsystem is destroyed
void UGfpmSwitcherEditorSubsystem::Deinitialize()
{
	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		if (RegisterTabsHandle.IsValid())
		{
			LevelEditorModule->OnRegisterTabs().Remove(RegisterTabsHandle);
		}
		if (LayoutExtensionHandle.IsValid())
		{
			LevelEditorModule->OnRegisterLayoutExtensions().Remove(LayoutExtensionHandle);
		}

		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
		if (LevelEditorTabManager.IsValid())
		{
			LevelEditorTabManager->UnregisterTabSpawner(GfpmSwitcherTab::TabName);
		}
	}

	if (GEditor
	    && BlueprintReinstancedHandle.IsValid())
	{
		GEditor->OnBlueprintReinstanced().Remove(BlueprintReinstancedHandle);
	}

	RegisterTabsHandle.Reset();
	LayoutExtensionHandle.Reset();
	BlueprintReinstancedHandle.Reset();

	SwitcherWidget = nullptr;

	Super::Deinitialize();
}
