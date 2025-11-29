// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"

// Bomber
#include "Structures/ManageableWidgetData.h"

// UE
#include "GameFeatureStateChangeObserver.h"

#include "WidgetsSubsystem.generated.h"

class UUserWidget;

struct FManageableWidgetData;

/**
 * Is used to manage User Widgets with lifetime of Local Player (similar to HUD).
 * @see Access its data with UUIDataAsset (Content/Bomber/DataAssets/DA_UI).
 */
UCLASS()
class BOMBER_API UWidgetsSubsystem : public ULocalPlayerSubsystem,
                                     public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/** Returns the pointer the UI Subsystem.
	 * It will return null if Local Player is not initialized yet. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UWidgetsSubsystem* GetWidgetsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/** Returns the UI subsystem checked: it will crash if player controller is not initialized yet.
	 * @warning don't call it on BeginPlay, do it not earlier than OnLocalCharacterReady */
	static UWidgetsSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Widgets Management
	 * Widgets using these methods are managed by this subsystem and can be controlled globally, e.g. hide/show all widgets.
	 ********************************************************************************************* */
public:
	/** Creates and registers specified widget to the Manageable widgets list, so its visibility can be changed globally. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	UUserWidget* CreateManageableWidget(const FManageableWidgetData& WidgetData, const UObject* OptionalWorldContext = nullptr);

	/** Is alternative to CreateManageableWidget, but with templated cast and crashes if widget class is not valid.
	 * E.g: UMyUserWidget* NewWidget = CreateManageableWidgetChecked<UMyUserWidget>(WidgetData); */
	template <typename T = UUserWidget>
	FORCEINLINE T& CreateManageableWidgetChecked(const FManageableWidgetData& WidgetData) { return *CastChecked<T>(CreateManageableWidget(WidgetData)); }

	/** The same as CreateManageableWidget, but finds widget data by tag from the UI Data Asset. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	UUserWidget* CreateManageableWidgetByTag(FGameplayTag WidgetTag, const UObject* OptionalWorldContext = nullptr);

	/** Is alternative to CreateManageableWidgetByTag, but with templated cast and crashes if widget class is not valid.
	 * E.g: UMyUserWidget* NewWidget = CreateManageableWidgetByTagChecked<UMyUserWidget>(WidgetTag); */
	template <typename T = UUserWidget>
	FORCEINLINE T& CreateManageableWidgetByTagChecked(FGameplayTag WidgetTag) { return *CastChecked<T>(CreateManageableWidgetByTag(WidgetTag)); }

	/** Returns the widget instance by its tag.
	 * @param WidgetTag - the tag associated with the widget to find.
	 * @param OptionalIndex - if there are multiple widgets with the same tag (like player nicknames), this index will specify which one to return: 0 by default. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	UUserWidget* GetWidgetByTag(FGameplayTag WidgetTag, int32 OptionalIndex = 0) const;

	/** Is alternative to GetManageableWidgetByTag, but with templated cast.
	 * E.g: const UMyUserWidget* FoundWidget = GetManageableWidgetByTagChecked<UMyUserWidget>(WidgetTag); */
	template <typename T = UUserWidget>
	FORCEINLINE T* GetWidgetByTag(FGameplayTag WidgetTag, int32 OptionalIndex = 0) const { return Cast<T>(GetWidgetByTag(WidgetTag, OptionalIndex)); }

	/** Returns all widgets associated with the given tag.
	 * @param WidgetTag - the tag associated with the widget to find, can partially match multiple tags.
	 * @param OutWidgets - the array to fill with found widgets, might be empty if nothing was found. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GetAllWidgetsByTag(FGameplayTag WidgetTag, TArray<UUserWidget*>& OutWidgets) const;

	/** Removes given widget from the list and destroys it by its tag. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DestroyManageableWidgetByTag(FGameplayTag WidgetTag);

protected:
	/** Contains all widgets that are managed by this subsystem. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "All Managable Widgets"))
	TMap<FGameplayTag, FBmrManageableWidgetsContainer> AllManageableWidgetsInternal;

	/*********************************************************************************************
	 * Core Widgets Initialization
	 * Widgets using there methods are initialized once when the game starts from the UI Data Asset.
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetsInitialized);

	/** Is called to notify that all widgets were initialized and ready. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnWidgetsInitialized OnWidgetsInitialized;

	/** Returns true if widgets ere initialized. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool AreWidgetInitialized() const { return bAreWidgetInitializedInternal; }

protected:
	/** Is true if widgets are initialized. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Are Widget Initialized"))
	bool bAreWidgetInitializedInternal = false;

	/** Will try to start the process of initializing all widgets used in game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryInitWidgets();

	/** Create and set widget objects once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitWidgets();

	/** Removes all widgets and transient data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void CleanupWidgets();

	/*********************************************************************************************
	 * Widgets Visibility
	 ********************************************************************************************* */
public:
	/** Is called to toggle all manageable widgets visibility.
	 * @param bMakeVisible - if true, changes all visible manageable widgets to hidden; false, restores visibility of all previously hidden widgets.
	 * @param bCanRestoreVisibilityLater - if true, original visibilities will be remembered, so they can be restored later if call this function again with reverse value. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAllWidgetsVisibility(bool bMakeVisible, bool bCanRestoreVisibilityLater = true);

	/** Returns true if all manageable widgets are hidden. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool AreAllWidgetsHidden() const { return !AllHiddenWidgetsInternal.IsEmpty(); }

protected:
	/** Contains widgets that globally were requested to hide, but were visible before, so their visibility will be restored when needed. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "All Hidden Widgets"))
	FGameplayTagContainer AllHiddenWidgetsInternal = FGameplayTagContainer::EmptyContainer;

	/*********************************************************************************************
	 * Overrides and Events
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

	/** Is called right after the game was started and windows size is set. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);
};