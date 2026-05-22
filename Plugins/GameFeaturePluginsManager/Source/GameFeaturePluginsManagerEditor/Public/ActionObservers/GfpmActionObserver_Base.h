// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Templates/SubclassOf.h" // TSubclassOf
#include "UObject/Object.h"

#include "GfpmActionObserver_Base.generated.h"

/**
 * Base for editor-only observers that hook engine editor logic on behalf of a Game Feature action type.
 * The owning subsystem creates one persistent instance per registered observed action and forwards the whole plugin lifecycle to it, so each observer reacts to exactly one action for the rest of the editor session.
 */
UCLASS(Abstract)
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmActionObserver_Base : public UObject
{
	GENERATED_BODY()

public:
	/** Identifies the action type this observer handles. */
	virtual TSubclassOf<class UGameFeatureAction> GetObservedActionClass() const PURE_VIRTUAL(UGfpmActionObserver_Base::GetObservedActionClass, return nullptr;);

	/** Whether this observer is created in current configuration, by default only in editor and not under -game. */
	virtual bool ShouldCreateObserver() const;

	/** Binds this observer to its observed action, called by the owning subsystem before the registering hook. */
	void SetObservedAction(class UGameFeatureAction* Action, const FString& InPluginURL);

	/** When owning plugin is registered and its observed action becomes available. */
	virtual void OnGameFeatureRegistering() { }

	/** When owning plugin begins activating, before its actions run. */
	virtual void OnGameFeatureActivating() { }

	/** When owning plugin finished activating, after its actions ran, the safe point to react to action state. */
	virtual void OnGameFeatureActivated() { }

	/** When owning plugin begins deactivating. */
	virtual void OnGameFeatureDeactivating() { }

	/** When owning plugin is unregistering and its observed action is about to be released. */
	virtual void OnGameFeatureUnregistering() { }

	/** Returns source plugin URL this observer reacts to. */
	const FString& GetPluginURL() const { return PluginURL; }

protected:
	/** Currently-registered observed action instance */
	TWeakObjectPtr<class UGameFeatureAction> ObservedAction = nullptr;

	/** Source plugin URL this observer reacts to */
	FString PluginURL;
};
