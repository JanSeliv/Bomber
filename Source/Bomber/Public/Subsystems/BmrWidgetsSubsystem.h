// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"

// Bomber
#include "Structures/BmrManageableWidgetData.h"

// UE
#include "GameFeatureStateChangeObserver.h"

#include "BmrWidgetsSubsystem.generated.h"

class UUserWidget;

struct FBmrManageableWidgetData;

/**
 * Is used to manage User Widgets with lifetime of Local Player (similar to HUD).
 * @see Access its data with UBmrUIDataAsset (Content/Bomber/DataAssets/DA_UI).
 */
UCLASS()
class BOMBER_API UBmrWidgetsSubsystem : public ULocalPlayerSubsystem
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/** Returns the pointer the UI Subsystem.
	 * It will return null if Local Player is not initialized yet. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UBmrWidgetsSubsystem* GetWidgetsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/** Returns the UI subsystem checked: it will crash if player controller is not initialized yet.
	 * @warning don't call it on BeginPlay, do it not earlier than OnLocalPawnReady */
	static UBmrWidgetsSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Widgets Management
	 * Widgets using these methods are managed by this subsystem and can be controlled globally, e.g. hide/show all widgets.
	 ********************************************************************************************* */
public:
	/** Creates and registers specified widget to the Manageable widgets list, so its visibility can be changed globally. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	UUserWidget* CreateManageableWidget(const FBmrManageableWidgetData& WidgetData, const UObject* OptionalWorldContext = nullptr);

	/** Is alternative to CreateManageableWidget, but with templated cast and crashes if widget class is not valid.
	 * E.g: UMyUserWidget* NewWidget = CreateManageableWidgetChecked<UMyUserWidget>(WidgetData); */
	template <typename T = UUserWidget>
	FORCEINLINE T& CreateManageableWidgetChecked(const FBmrManageableWidgetData& WidgetData) { return *CastChecked<T>(CreateManageableWidget(WidgetData)); }

	/** The same as CreateManageableWidget, but finds widget data by tag from the UI Data Asset. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	UUserWidget* CreateManageableWidgetByTag(FGameplayTag WidgetTag, const UObject* OptionalWorldContext = nullptr);

	/** Is alternative to CreateManageableWidgetByTag, but with templated cast and crashes if widget class is not valid.
	 * E.g: UMyUserWidget* NewWidget = CreateManageableWidgetByTagChecked<UMyUserWidget>(WidgetTag); */
	template <typename T = UUserWidget>
	FORCEINLINE T& CreateManageableWidgetByTagChecked(FGameplayTag WidgetTag) { return *CastChecked<T>(CreateManageableWidgetByTag(WidgetTag)); }

	/** Returns the widget instance by its tag.
	 * @param WidgetTag - the tag associated with the widget to find.
	 * @param OptionalIndex - if there are multiple widgets with the same tag (like player nicknames), this index will specify which one to return: 0 by default. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	UUserWidget* GetWidgetByTag(FGameplayTag WidgetTag, int32 OptionalIndex = 0) const;

	/** Is alternative to GetManageableWidgetByTag, but with templated cast.
	 * E.g: const UMyUserWidget* FoundWidget = GetManageableWidgetByTagChecked<UMyUserWidget>(WidgetTag); */
	template <typename T = UUserWidget>
	FORCEINLINE T* GetWidgetByTag(FGameplayTag WidgetTag, int32 OptionalIndex = 0) const { return Cast<T>(GetWidgetByTag(WidgetTag, OptionalIndex)); }

	/** Returns all widgets associated with the given tag.
	 * @param WidgetTag - the tag associated with the widget to find, can partially match multiple tags.
	 * @param OutWidgets - the array to fill with found widgets, might be empty if nothing was found. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void GetAllWidgetsByTag(FGameplayTag WidgetTag, TArray<UUserWidget*>& OutWidgets) const;

	/** Removes given widget from the list and destroys it by its tag. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void DestroyManageableWidgetByTag(FGameplayTag WidgetTag);

protected:
	/** Contains all widgets that are managed by this subsystem. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TMap<FGameplayTag, FBmrManageableWidgetsContainer> AllManageableWidgets;

	/*********************************************************************************************
	 * Core Widgets Initialization
	 * Widgets using there methods are initialized once when the game starts from the UI Data Asset.
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetsInitialized);

	/** Is called to notify that all widgets were initialized and ready.
	 * In code, can use BIND_ON_WIDGETS_INITIALIZED(this, ThisClass::OnWidgetsInitialized); */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnWidgetsInitialized OnWidgetsInitialized;

	/** Returns true if widgets ere initialized. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool AreWidgetInitialized() const { return bAreWidgetInitialized; }

protected:
	/** Is true if widgets are initialized. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bAreWidgetInitialized = false;

	/** Will try to start the process of initializing all widgets used in game. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TryInitWidgets();

	/** Marks widgets as initialized and broadcasts OnWidgetsInitialized once, no-op on subsequent calls. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void InitWidgets();

	/** Creates widgets for all Data Registry rows not yet present in AllManageableWidgets. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void CreateMissingWidgets();

	/** Removes all widgets and transient data. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void CleanupWidgets();

	/*********************************************************************************************
	 * Widgets Visibility
	 ********************************************************************************************* */
public:
	/** Is called to toggle all manageable widgets visibility.
	 * @param bMakeVisible - if true, changes all visible manageable widgets to hidden; false, restores visibility of all previously hidden widgets.
	 * @param bCanRestoreVisibilityLater - if true, original visibilities will be remembered, so they can be restored later if call this function again with reverse value. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetAllWidgetsVisibility(bool bMakeVisible, bool bCanRestoreVisibilityLater = true);

	/** Returns true if all manageable widgets are hidden. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool AreAllWidgetsHidden() const { return !AllHiddenWidgets.IsEmpty(); }

protected:
	/** Contains widgets that globally were requested to hide, but were visible before, so their visibility will be restored when needed. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FGameplayTagContainer AllHiddenWidgets = FGameplayTagContainer::EmptyContainer;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is overridden to perform initial bindings (however, is too early to init widgets here until controller ready). */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Is overridden to cleanup injected widgets to let them unload properly. */
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override;

	/** Callback for when the player controller is changed on this subsystem's owning local player. */
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	/** Is called when this Subsystem is removed. */
	virtual void Deinitialize() override;

	/** Is called when the owning local player's world begins play. */
	void OnBeginPlay(UWorld* World, struct FWorldInitializationValues WorldInitializationValues);

	/** Is called when the owning local player's world is being cleaned up. */
	void OnEndPlay(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called right after the game was started and windows size is set. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);

	/** Called after widget Data Registry rows change and all new soft references finish async loading */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnWidgetRowsChanged();
};

/** Helper macro to bind and call the function when widgets are initialized */
#define BIND_ON_WIDGETS_INITIALIZED(Obj, Function)                                                \
	{                                                                                             \
		if (UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem()) \
		{                                                                                         \
			WidgetsSubsystem->OnWidgetsInitialized.AddUniqueDynamic(Obj, &Function);              \
			if (WidgetsSubsystem->AreWidgetInitialized())                                         \
			{                                                                                     \
				Obj->Function();                                                                  \
			}                                                                                     \
		}                                                                                         \
	}