// Copyright (c) Yevhenii Selivanov.

#include "Subsystems/BmrWidgetsSubsystem.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "DalRegistrySubsystem.h"
#include "DataRegistries/BmrWidgetRow.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/Viewport.h"
#include "Engine/World.h"
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

// The same as CreateManageableWidget, but finds widget data by tag from Data Registry
UUserWidget* UBmrWidgetsSubsystem::CreateManageableWidgetByTag(FGameplayTag WidgetTag, const UObject* OptionalWorldContext)
{
	const FBmrWidgetRow* WidgetRowData = FBmrWidgetRow::GetRowByTag(WidgetTag);
	if (!WidgetRowData)
	{
		return nullptr;
	}

	UClass* WidgetClass = WidgetRowData->WidgetClass.Get();
	if (!ensureMsgf(WidgetClass, TEXT("ASSERT: [%i] %hs:\n'WidgetClass' is not loaded for widget tag '%s'!"), __LINE__, __FUNCTION__, *WidgetTag.ToString()))
	{
		return nullptr;
	}

	FBmrManageableWidgetData WidgetData;
	WidgetData.WidgetClass = WidgetClass;
	WidgetData.WidgetTag = WidgetRowData->WidgetTag;
	WidgetData.bAddToViewport = WidgetRowData->bAddToViewport;
	WidgetData.ZOrder = WidgetRowData->ZOrder;
	return CreateManageableWidget(WidgetData, OptionalWorldContext);
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

// Creates widgets for all Data Registry rows not yet present in AllManageableWidgets
void UBmrWidgetsSubsystem::CreateMissingWidgets()
{
	FBmrWidgetRow::ForEachRow([this](const FBmrWidgetRow& WidgetRowData)
	{
		if (AllManageableWidgets.Contains(WidgetRowData.WidgetTag))
		{
			return;
		}

		UClass* WidgetClass = WidgetRowData.WidgetClass.Get();
		if (!ensureMsgf(WidgetClass, TEXT("ASSERT: [%i] %hs:\n'WidgetClass' is not loaded for widget tag '%s'!"), __LINE__, __FUNCTION__, *WidgetRowData.WidgetTag.ToString()))
		{
			return;
		}

		FBmrManageableWidgetData WidgetData;
		WidgetData.WidgetClass = WidgetClass;
		WidgetData.WidgetTag = WidgetRowData.WidgetTag;
		WidgetData.bAddToViewport = WidgetRowData.bAddToViewport;
		WidgetData.ZOrder = WidgetRowData.ZOrder;
		CreateManageableWidget(WidgetData);
	});
}

// Marks widgets as initialized and broadcasts OnWidgetsInitialized once, no-op on subsequent calls
void UBmrWidgetsSubsystem::InitWidgets()
{
	if (AreWidgetInitialized())
	{
		return;
	}

	CreateMissingWidgets();

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

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnBeginPlay);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnEndPlay);
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

	UDalRegistrySubsystem::Get().BindAndLoad<FBmrWidgetRow>(this, &ThisClass::OnWidgetRowsChanged);
}

// Is called when this Subsystem is removed
void UBmrWidgetsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
}

// Is overridden to perform initial bindings (however, is too early to init widgets here until controller ready)
void UBmrWidgetsSubsystem::OnBeginPlay(UWorld* World, FWorldInitializationValues WorldInitializationValues)
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer
	    || LocalPlayer->GetWorld() != World)
	{
		return;
	}

	UGameFeaturesSubsystem::Get().AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Unbinds DAL and removes the game feature observer on world teardown so streamable references release and widgets are cleaned up
void UBmrWidgetsSubsystem::OnEndPlay(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer
	    || LocalPlayer->GetWorld() != World)
	{
		return;
	}

	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->UnbindFromDataRegistryLoad(this);
	}

	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	CleanupWidgets();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called right after the game was started and windows size is set
void UBmrWidgetsSubsystem::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	InitWidgets();
}

// Called after widget Data Registry rows change and all new soft references finish async loading
void UBmrWidgetsSubsystem::OnWidgetRowsChanged_Implementation()
{
	if (!AreWidgetInitialized())
	{
		// Widget rows just became available in DR, trigger initial widget creation respecting viewport readiness
		TryInitWidgets();
		return;
	}

	// Gather current DR row tags to diff against tracked state
	TSet<FGameplayTag> CurrentTags;
	FBmrWidgetRow::ForEachRow([&CurrentTags](const FBmrWidgetRow& WidgetRowData)
	{
		CurrentTags.Add(WidgetRowData.WidgetTag);
	});

	// REMOVAL: destroy widgets whose tags vanished from DR
	TArray<FGameplayTag> TagsToRemove;
	for (const TPair<FGameplayTag, FBmrManageableWidgetsContainer>& It : AllManageableWidgets)
	{
		if (!CurrentTags.Contains(It.Key))
		{
			TagsToRemove.Add(It.Key);
		}
	}
	for (const FGameplayTag& Tag : TagsToRemove)
	{
		DestroyManageableWidgetByTag(Tag);
	}

	// ADDITION: create widgets for newly injected rows, assets are already loaded by the handle
	CreateMissingWidgets();
}
