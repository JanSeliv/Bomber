// Copyright (c) Yevhenii Selivanov.

#include "Subsystems/WidgetsSubsystem.h"

// Bomber
#include "Controllers/MyPlayerController.h"
#include "DataAssets/UIDataAsset.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

// UE
#include "Components/Viewport.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "Misc/PackageName.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(WidgetsSubsystem)

// Returns the pointer the UI Subsystem
UWidgetsSubsystem* UWidgetsSubsystem::GetWidgetsSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(OptionalWorldContext);
	if (!LocalPlayer)
	{
		const AMyPlayerController* PC = Cast<AMyPlayerController>(OptionalWorldContext);
		if (!PC)
		{
			PC = UMyBlueprintFunctionLibrary::GetLocalPlayerController(OptionalWorldContext);
		}
		LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	}
	return LocalPlayer ? LocalPlayer->GetSubsystem<UWidgetsSubsystem>() : nullptr;
}

// Returns the UI subsystem checked: it will crash if player controller is not initialized yet
UWidgetsSubsystem& UWidgetsSubsystem::Get(const UObject* OptionalWorldContext)
{
	UWidgetsSubsystem* WidgetsSubsystem = GetWidgetsSubsystem(OptionalWorldContext);
	checkf(WidgetsSubsystem, TEXT("%s: 'WidgetsSubsystem' is null, likely controller is not initialized yet!"), *FString(__FUNCTION__));
	return *WidgetsSubsystem;
}

/*********************************************************************************************
 * Widgets Management
 ********************************************************************************************* */

// Create specified widget and add it to Manageable widgets list, so its visibility can be changed globally
UUserWidget* UWidgetsSubsystem::CreateManageableWidget(const FManageableWidgetData& WidgetData, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (!ensureMsgf(WidgetData.IsValid(), TEXT("ASSERT: [%i] %hs:\n'WidgetData' is not valid, likely not set in the UI Data Asset: %s"), __LINE__, __FUNCTION__, *WidgetData.ToString()))
	{
		return nullptr;
	}

	UUserWidget* Widget = FWidgetUtilsLibrary::CreateWidgetByClass(WidgetData.WidgetClass, WidgetData.bAddToViewport, WidgetData.ZOrder, OptionalWorldContext);
	FBmrManageableWidgetsContainer& WidgetsContainer = AllManageableWidgetsInternal.FindOrAdd(WidgetData.WidgetTag);
	WidgetsContainer.WidgetInstances.Add(Widget);

	return Widget;
}

// The same as CreateManageableWidget, but finds widget data by tag from the UI Data Asset
UUserWidget* UWidgetsSubsystem::CreateManageableWidgetByTag(FGameplayTag WidgetTag, const UObject* OptionalWorldContext)
{
	return CreateManageableWidget(UUIDataAsset::Get().GetWidgetDataByTag(WidgetTag), OptionalWorldContext);
}

// Returns the widget instance by its tag
UUserWidget* UWidgetsSubsystem::GetWidgetByTag(FGameplayTag WidgetTag, int32 OptionalIndex /* = 0*/) const
{
	const FBmrManageableWidgetsContainer* WidgetsContainer = AllManageableWidgetsInternal.Find(WidgetTag);
	if (!WidgetsContainer
	    || !WidgetsContainer->WidgetInstances.IsValidIndex(OptionalIndex))
	{
		return nullptr;
	}

	return WidgetsContainer->WidgetInstances[OptionalIndex].Get();
}

// Returns all widgets associated with the given tag
void UWidgetsSubsystem::GetAllWidgetsByTag(FGameplayTag WidgetTag, TArray<UUserWidget*>& OutWidgets) const
{
	for (const TPair<FGameplayTag, FBmrManageableWidgetsContainer>& PairIt : AllManageableWidgetsInternal)
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
void UWidgetsSubsystem::DestroyManageableWidgetByTag(FGameplayTag WidgetTag)
{
	if (!ensureMsgf(WidgetTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'WidgetTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	FBmrManageableWidgetsContainer* WidgetsContainer = AllManageableWidgetsInternal.Find(WidgetTag);
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

	AllManageableWidgetsInternal.Remove(WidgetTag);

	AllHiddenWidgetsInternal.RemoveTag(WidgetTag);
}

/*********************************************************************************************
 * Core Widgets Initialization
 ********************************************************************************************* */

// Will try to start the process of initializing all widgets used in game
void UWidgetsSubsystem::TryInitWidgets()
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
void UWidgetsSubsystem::InitWidgets()
{
	if (AreWidgetInitialized())
	{
		return;
	}

	const TArray<FManageableWidgetData>& AllWidgetData = UUIDataAsset::Get().GetAllWidgetData();
	for (const FManageableWidgetData& WidgetDataIt : AllWidgetData)
	{
		// Automatically create and add all widgets to the viewport from the UI Data Asset
		CreateManageableWidget(WidgetDataIt);
	}

	bAreWidgetInitializedInternal = true;

	if (OnWidgetsInitialized.IsBound())
	{
		OnWidgetsInitialized.Broadcast();
	}
}

// Removes all widgets and transient data
void UWidgetsSubsystem::CleanupWidgets()
{
	while (!AllManageableWidgetsInternal.IsEmpty())
	{
		const FGameplayTag WidgetTag = AllManageableWidgetsInternal.CreateIterator().Key();
		DestroyManageableWidgetByTag(WidgetTag);
		AllManageableWidgetsInternal.Remove(WidgetTag);
	}

	AllManageableWidgetsInternal.Empty();
	AllHiddenWidgetsInternal = FGameplayTagContainer::EmptyContainer;

	bAreWidgetInitializedInternal = false;
}

/*********************************************************************************************
 * Widgets Visibility
 ********************************************************************************************* */

// If true, changes all visible manageable widgets to hidden
void UWidgetsSubsystem::SetAllWidgetsVisibility(bool bMakeVisible, bool bCanRestoreVisibilityLater /* = true*/)
{
	const ESlateVisibility DesiredVisibility = bMakeVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const FGameplayTagContainer TagsToProcess = [&]()
	{
		FGameplayTagContainer Result = FGameplayTagContainer::EmptyContainer;
		if (bMakeVisible)
		{
			Result = AllHiddenWidgetsInternal;
		}
		else
		{
			TArray<FGameplayTag> AllTags;
			AllManageableWidgetsInternal.GenerateKeyArray(AllTags);
			Result = FGameplayTagContainer::CreateFromArray(AllTags);
		}
		return Result;
	}();

	if (!bMakeVisible)
	{
		AllHiddenWidgetsInternal = FGameplayTagContainer::EmptyContainer;
	}

	for (const FGameplayTag& WidgetTag : TagsToProcess)
	{
		const FBmrManageableWidgetsContainer* Container = AllManageableWidgetsInternal.Find(WidgetTag);
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
					AllHiddenWidgetsInternal.AddTag(WidgetTag);
				}
			}
		}
	}

	if (bMakeVisible)
	{
		AllHiddenWidgetsInternal = FGameplayTagContainer::EmptyContainer;
	}
}

/*********************************************************************************************
 * Overrides and Events
 ********************************************************************************************* */

// Is overridden to perform initial bindings (however, is too early to init widgets here until controller ready)
void UWidgetsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Is overridden to cleanup injected widgets to let them unload properly
void UWidgetsSubsystem::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	checkf(GameFeatureData, TEXT("ERROR: [%i] %hs:\n'GameFeatureData' is null!"), __LINE__, __FUNCTION__);

	FString OutModuleRootPath;
	FString OutPackagePath;
	FString OutPackageName;
	FPackageName::SplitLongPackageName(GameFeatureData->GetPathName(), OutModuleRootPath, OutPackagePath, OutPackageName);

	FGameplayTagContainer WidgetsOwnedByModule = FGameplayTagContainer::EmptyContainer;
	for (const TPair<FGameplayTag, FBmrManageableWidgetsContainer>& It : AllManageableWidgetsInternal)
	{
		for (const UUserWidget* WidgetInstanceIt : It.Value.WidgetInstances)
		{
			const TSubclassOf<UUserWidget> WidgetClassIt = WidgetInstanceIt ? WidgetInstanceIt->GetClass() : nullptr;
			if (!WidgetClassIt)
			{
				continue;
			}

			FString OutWidgetRootPath;
			FString OutWidgetPath;
			FString OutWidgetName;
			FPackageName::SplitLongPackageName(WidgetClassIt->GetPathName(), OutWidgetRootPath, OutWidgetPath, OutWidgetName);
			if (OutWidgetRootPath == OutModuleRootPath)
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
void UWidgetsSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	const AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayerController);
	if (!MyPC
	    || MyPC->bIsDebugCameraEnabledInternal)
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
void UWidgetsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	CleanupWidgets();
}

// Is called right after the game was started and windows size is set
void UWidgetsSubsystem::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	InitWidgets();
}