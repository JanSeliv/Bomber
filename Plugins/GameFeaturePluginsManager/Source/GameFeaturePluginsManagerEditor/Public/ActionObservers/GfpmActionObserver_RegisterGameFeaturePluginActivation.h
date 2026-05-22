// Copyright (c) Yevhenii Selivanov

#pragma once

#include "ActionObservers/GfpmActionObserver_Base.h"

#include "GfpmActionObserver_RegisterGameFeaturePluginActivation.generated.h"

/**
 * Editor-only observer that keeps the tag-driven activation registry the same across every live world's loader.
 * Builds run a single world the runtime action feeds directly, while PIE runs many worlds at once that each need the same registry.
 */
UCLASS()
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmActionObserver_RegisterGameFeaturePluginActivation final : public UGfpmActionObserver_Base
{
	GENERATED_BODY()

public:
	/** Identifies the action type this observer handles. */
	virtual TSubclassOf<class UGameFeatureAction> GetObservedActionClass() const override;

	/** When owning plugin is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** When owning plugin is unregistering. */
	virtual void OnGameFeatureUnregistering() override;

protected:
	/** Mirrors observed action's activation entry into every live world's tag-driven loader, or drops it when bRegister is false. */
	void MirrorAcrossWorlds(bool bRegister) const;
};
