// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayEffectComponent.h"

// UE
#include "ActiveGameplayEffectHandle.h"

#include "EndWithOwnAbilityGameplayEffectComponent.generated.h"

/**
 * Is responsible for sharing lifetime between this effect and ability that applied it, so either one ending takes other with it.
 */
UCLASS(CollapseCategories, DisplayName = "End With Own Ability")
class MYUTILS_API UEndWithOwnAbilityGameplayEffectComponent : public UGameplayEffectComponent
{
	GENERATED_BODY()

public:
	/** Is overridden to bind both lifetimes on server: ability end removes this effect, effect end cancels that ability. */
	virtual bool OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const override;

protected:
	/** Called when this effect is removed, cancels ability that applied it and releases own ability end listener. */
	void OnActiveGameplayEffectRemoved(const struct FGameplayEffectRemovalInfo& RemovalInfo, FDelegateHandle AbilityEndedHandle) const;

	/** Called when any ability on same Ability System Component ends, removes this effect once that ability is one that applied it. */
	void OnAbilityEnded(const struct FAbilityEndedData& EndedData, TWeakObjectPtr<const class UGameplayAbility> WeakAbility, FActiveGameplayEffectHandle EffectHandle) const;
};
