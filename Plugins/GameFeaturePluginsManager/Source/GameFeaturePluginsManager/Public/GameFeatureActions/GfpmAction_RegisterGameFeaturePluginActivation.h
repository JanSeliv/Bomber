// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "GameplayTagContainer.h"

#include "GfpmAction_RegisterGameFeaturePluginActivation.generated.h"

/**
 * Enables owning Game Feature Plugin to auto-activate while declared tags are present on world Ability System Component, deactivate when none remain.
 */
UCLASS(DisplayName = "GFPM Register Game Feature Plugin Activation")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAction_RegisterGameFeaturePluginActivation final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** Tags that trigger activation of owning plugin when any of them appears on authoritative ASC. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]")
	FGameplayTagContainer ActivationTags = FGameplayTagContainer::EmptyContainer;

protected:
	/** Called by Game Features system when owning plugin is registered. */
	virtual void OnGameFeatureRegistering() override;

	/** Called by Game Features system when owning plugin is unregistered. */
	virtual void OnGameFeatureUnregistering() override;

	/** When any world finished initializing. */
	void OnPostWorldInitialized(class UWorld* World);

	/** Subscription handle feeding each world's loader as it initializes. */
	FDelegateHandle OnPostWorldInitHandle;

#if WITH_EDITOR
	/** Called by editor's Data Validation system when validation runs on this object. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;

	/** Called by editor when any property on this object is changed in details panel. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
