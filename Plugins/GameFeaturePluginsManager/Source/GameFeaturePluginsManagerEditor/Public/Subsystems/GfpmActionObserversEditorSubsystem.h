// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureStateChangeObserver.h" // IGameFeatureStateChangeObserver
#include "Subsystems/EngineSubsystem.h"

#include "GfpmActionObserversEditorSubsystem.generated.h"

class UGameFeatureData;

/**
 * Owns one persistent observer per registered Game Feature action.
 * Acts as the single game-feature state-change observer, creating each observer when its plugin is registered and forwarding the whole plugin lifecycle to it for the rest of the session.
 * Engine-scoped so it runs in -game as well (which is also treated as editor configuration)
 */
UCLASS(DisplayName = "Game Feature Plugins Action Observers Editor Subsystem")
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmActionObserversEditorSubsystem : public UEngineSubsystem
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/** When subsystem initializes. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** When subsystem is destroyed. */
	virtual void Deinitialize() override;

	/** When a game feature plugin is registered. */
	virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;

	/** When a game feature plugin begins transition into Active state. */
	virtual void OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/** When a game feature plugin finished transition into Active state. */
	virtual void OnGameFeatureActivated(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/** When a game feature plugin transitions out of Active state. */
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, struct FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override;

	/** When a game feature plugin is unregistering. */
	virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;

protected:
	/** Persistent observers, one per registered observed action, never pruned for the editor session */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Transient, Category = "[Game Feature Plugins Manager Editor]")
	TArray<TObjectPtr<class UGfpmActionObserver_Base>> Observers;

	/** Returns observer of given class bound to given plugin URL, creating and storing it on first request.
	 * @param PluginURL Source plugin URL the observer reacts to.
	 * @param ObserverClass Concrete observer class to find or instantiate. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	class UGfpmActionObserver_Base* FindOrCreateObserver(const FString& PluginURL, TSubclassOf<class UGfpmActionObserver_Base> ObserverClass);
};
