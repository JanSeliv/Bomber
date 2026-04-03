// Copyright (c) Yevhenii Selivanov.

#include "Subsystems/BmrWidgetsSubsystem.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrUIDataAsset.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/Viewport.h"
#include "GameFeaturesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrWidgetsSubsystem)

// Returns the pointer the UI Subsystem
UBmrWidgetsSubsystem* UBmrWidgetsSubsystem::GetWidgetsSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(OptionalWorldContext);
	if (!LocalPlayer)
	{
		const ABmrPlayerController* PC = Cast<ABmrPlayerController>(OptionalWorldContext);
		if (!PC)
		{
			PC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController(OptionalWorldContext);
		}
		LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	}
	return LocalPlayer ? LocalPlayer->GetSubsystem<UBmrWidgetsSubsystem>() : nullptr;
}

// Returns the UI subsystem checked: it will crash if player controller is not initialized yet
UBmrWidgetsSubsystem& UBmrWidgetsSubsystem::Get(const UObject* OptionalWorldContext)
{
	UBmrWidgetsSubsystem* WidgetsSubsystem = GetWidgetsSubsystem(OptionalWorldContext);
	checkf(WidgetsSubsystem, TEXT("%s: 'WidgetsSubsystem' is null, likely controller is not initialized yet!"), *FString(__FUNCTION__));
	return *WidgetsSubsystem;
}

/*********************************************************************************************
 * Widgets Management
 ********************************************************************************************* */

// Create specified widget and add it to Manageable widgets list, so its visibility can be changed globally
UUserWidget* UBmrWidgetsSubsystem::CreateManageableWidget(const FBmrManageableWidgetData& WidgetData, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (!ensureMsgf(WidgetData.IsValid(), TEXT("ASSERT: [%i] %hs:\n'WidgetData' is not valid, likely not set in the UI Data Asset: %s"), __LINE__, __FUNCTION__, *WidgetData.ToString()))
	{
		return nullptr;
	}

	UUserWidget* Widget = FWidgetUtilsLibrary::CreateWidgetByClass(WidgetData.WidgetClass, WidgetData.bAddToViewport, WidgetData.ZOrder, OptionalWorldContext);
	FBmrManageableWidgetsContainer& WidgetsContainer = AllManageableWidgets.FindOrAdd(WidgetData.WidgetTag);
	WidgetsContainer.WidgetInstances.Add(Widget);

	return Widget;
}

// The same as CreateManageableWidget, but finds widget data by tag from the UI Data Asset
UUserWidget* UBmrWidgetsSubsystem::CreateManageableWidgetByTag(FGameplayTag WidgetTag, const UObject* OptionalWorldContext)
{
	return CreateManageableWidget(UBmrUIDataAsset::Get().GetWidgetDataByTag(WidgetTag), OptionalWorldContext);
}

// Returns the widget instance by its tag
UUserWidget* UBmrWidgetsSubsystem::GetWidgetByTag(FGameplayTag WidgetTag, int32 OptionalIndex /* = 0*/) const
{
	const FBmrManageableWidgetsContainer* WidgetsContainer = AllManageableWidgets.Find(WidgetTag);
	if (!WidgetsContainer
	    || !WidgetsContainer->WidgetInstances.IsValidIndex(OptionalIndex))
	{
		return nullptr;
	}

	return WidgetsContainer->WidgetInstances[OptionalIndex].Get();
}

// Returns all widgets associated with the given tag
void UBmrWidgetsSubsystem::GetAllWidgetsByTag(FGameplayTag WidgetTag, TArray<UUserWidget*>& OutWidgets) const
{
	for (const TPair<FGameplayTag, FBmrManageableWidgetsContainer>& PairIt : AllManageableWidgets)
	{
		if (PairIt.Key.MatchesTag(WidgetTag))
		{
			for (UUserWidget* Widget : PairIt.Value.WidgetInstances)
			{
				if (Widget)
				{
					OutWidgets.Add(Widget);
				}
			}
		}
	}
}

// Removes given widget from the list and destroys it by its tag
void UBmrWidgetsSubsystem::DestroyManageableWidgetByTag(FGameplayTag WidgetTag)
{
	if (!ensureMsgf(WidgetTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'WidgetTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	FBmrManageableWidgetsContainer* WidgetsContainer = AllManageableWidgets.Find(WidgetTag);
	if (!WidgetsContainer)
	{
		// Such widget tag is not even registered
		return;
	}

	// The same tag might be associated with multiple widgets (like player nicknames), so destroy them all
	for (int32 Index = WidgetsContainer->WidgetInstances.Num() - 1; Index >= 0; --Index)
	{
		if (UUserWidget* WidgetIt = WidgetsContainer->WidgetInstances[Index].Get())
		{
			FWidgetUtilsLibrary::DestroyWidget(*WidgetIt);
		}

		WidgetsContainer->WidgetInstances.RemoveAtSwap(Index);
	}

	AllManageableWidgets.Remove(WidgetTag);

	AllHiddenWidgets.RemoveTag(WidgetTag);
}

/*********************************************************************************************
 * Core Widgets Initialization
 ********************************************************************************************* */

// Will try to start the process of initializing all widgets used in game
void UBmrWidgetsSubsystem::TryInitWidgets()
{
	if (UUtilsLibrary::IsViewportInitialized())
	{
		InitWidgets();
	}
	else if (!FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewportResizedWhenInit);
	}
}

// Create and set widget objects once
void UBmrWidgetsSubsystem::InitWidgets()
{
	if (AreWidgetInitialized())
	{
		return;
	}

	const TArray<FBmrManageableWidgetData>& AllWidgetData = UBmrUIDataAsset::Get().GetAllWidgetData();
	for (const FBmrManageableWidgetData& WidgetDataIt : AllWidgetData)
	{
		// Automatically create and add all widgets to the viewport from the UI Data Asset
		CreateManageableWidget(WidgetDataIt);
	}

	bAreWidgetInitialized = true;

	if (OnWidgetsInitialized.IsBound())
	{
		OnWidgetsInitialized.Broadcast();
	}
}

// Removes all widgets and transient data
void UBmrWidgetsSubsystem::CleanupWidgets()
{
	while (!AllManageableWidgets.IsEmpty())
	{
		const FGameplayTag WidgetTag = AllManageableWidgets.CreateIterator().Key();
		DestroyManageableWidgetByTag(WidgetTag);
		AllManageableWidgets.Remove(WidgetTag);
	}

	AllManageableWidgets.Empty();
	AllHiddenWidgets = FGameplayTagContainer::EmptyContainer;

	bAreWidgetInitialized = false;
}

/*********************************************************************************************
 * Widgets Visibility
 ********************************************************************************************* */

// If true, changes all visible manageable widgets to hidden
void UBmrWidgetsSubsystem::SetAllWidgetsVisibility(bool bMakeVisible, bool bCanRestoreVisibilityLater /* = true*/)
{
	const ESlateVisibility DesiredVisibility = bMakeVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const FGameplayTagContainer TagsToProcess = [&]()
	{
		FGameplayTagContainer Result = FGameplayTagContainer::EmptyContainer;
		if (bMakeVisible)
		{
			Result = AllHiddenWidgets;
		}
		else
		{
			TArray<FGameplayTag> AllTags;
			AllManageableWidgets.GenerateKeyArray(AllTags);
			Result = FGameplayTagContainer::CreateFromArray(AllTags);
		}
		return Result;
	}();

	if (!bMakeVisible)
	{
		AllHiddenWidgets = FGameplayTagContainer::EmptyContainer;
	}

	for (const FGameplayTag& WidgetTag : TagsToProcess)
	{
		const FBmrManageableWidgetsContainer* Container = AllManageableWidgets.Find(WidgetTag);
		if (!Container)
		{
			continue;
		}

		for (UUserWidget* Widget : Container->WidgetInstances)
		{
			if (Widget && Widget->GetVisibility() != DesiredVisibility)
			{
				Widget->SetVisibility(DesiredVisibility);

				if (!bMakeVisible && bCanRestoreVisibilityLater)
				{
					AllHiddenWidgets.AddTag(WidgetTag);
				}
			}
		}
	}

	if (bMakeVisible)
	{
		AllHiddenWidgets = FGameplayTagContainer::EmptyContainer;
	}
}

/*********************************************************************************************
 * Overrides and Events
 ********************************************************************************************* */

// Is overridden to perform initial bindings (however, is too early to init widgets here until controller ready)
void UBmrWidgetsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Is overridden to cleanup injected widgets to let them unload properly
void UBmrWidgetsSubsystem::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	checkf(GameFeatureData, TEXT("ERROR: [%i] %hs:\n'GameFeatureData' is null!"), __LINE__, __FUNCTION__);

	FGameplayTagContainer WidgetsOwnedByModule = FGameplayTagContainer::EmptyContainer;
	for (const TPair<FGameplayTag, FBmrManageableWidgetsContainer>& It : AllManageableWidgets)
	{
		for (const UUserWidget* WidgetInstanceIt : It.Value.WidgetInstances)
		{
			const TSubclassOf<UUserWidget> WidgetClassIt = WidgetInstanceIt ? WidgetInstanceIt->GetClass() : nullptr;
			if (UModularGameFeaturePluginUtils::IsInGameFeatureModule(WidgetClassIt, GameFeatureData))
			{
				WidgetsOwnedByModule.AddTagFast(It.Key);
				break;
			}
		}
	}

	// Destroy all widgets that were created by this game feature module
	for (const FGameplayTag& WidgetTagIt : WidgetsOwnedByModule)
	{
		DestroyManageableWidgetByTag(WidgetTagIt);
	}
}

// Callback for when the player controller is changed on this subsystem's owning local player
void UBmrWidgetsSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	const ABmrPlayerController* MyPC = Cast<ABmrPlayerController>(NewPlayerController);
	if (!MyPC
	    || MyPC->bIsDebugCameraEnabled)
	{
		// Do not initialize widgets if different controller is possessed, likely Debug Controller, or Debug Camera is enabled
		return;
	}

	if (AreWidgetInitialized())
	{
		// New player controller is set, likely level was changed, so perform cleanup first
		CleanupWidgets();
	}

	TryInitWidgets();
}

// Is called when this Subsystem is removed
void UBmrWidgetsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	CleanupWidgets();
}

// Is called right after the game was started and windows size is set
void UBmrWidgetsSubsystem::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	InitWidgets();
}