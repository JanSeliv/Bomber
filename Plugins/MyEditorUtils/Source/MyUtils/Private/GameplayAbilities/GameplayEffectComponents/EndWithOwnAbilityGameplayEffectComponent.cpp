// Copyright (c) Yevhenii Selivanov

#include "GameplayAbilities/GameplayEffectComponents/EndWithOwnAbilityGameplayEffectComponent.h"

// UE
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EndWithOwnAbilityGameplayEffectComponent)

// Is overridden to bind both lifetimes on server: ability end removes this effect, effect end cancels that ability
bool UEndWithOwnAbilityGameplayEffectComponent::OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const
{
	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	const UGameplayAbility* InstigatorAbility = ActiveGE.Spec.GetEffectContext().GetAbilityInstance_NotReplicated();
	if (!ActiveGEContainer.IsNetAuthority()
	    || !ASC
	    || !InstigatorAbility)
	{
		// Effect removal is never predicted, so it always starts on server and both ends reach client through own replication, and effect applied outside any ability has none to share with
		return true;
	}

	const FDelegateHandle AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded, TWeakObjectPtr<const UGameplayAbility>(InstigatorAbility), ActiveGE.Handle);
	ActiveGE.EventSet.OnEffectRemoved.AddUObject(this, &ThisClass::OnActiveGameplayEffectRemoved, AbilityEndedHandle);

	return true;
}

// Called when this effect is removed, cancels ability that applied it and releases own ability end listener
void UEndWithOwnAbilityGameplayEffectComponent::OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo, FDelegateHandle AbilityEndedHandle) const
{
	UAbilitySystemComponent* ASC = RemovalInfo.OwningASC;
	if (!ASC)
	{
		// Own Ability System Component can be gone already on teardown
		return;
	}

	ASC->OnAbilityEnded.Remove(AbilityEndedHandle);

	if (const UGameplayAbility* InstigatorAbility = RemovalInfo.EffectContext.GetAbilityInstance_NotReplicated())
	{
		ASC->CancelAbilityHandle(InstigatorAbility->GetCurrentAbilitySpecHandle());
	}
}

// Called when any ability on same Ability System Component ends, removes this effect once that ability is one that applied it
void UEndWithOwnAbilityGameplayEffectComponent::OnAbilityEnded(const FAbilityEndedData& EndedData, TWeakObjectPtr<const UGameplayAbility> WeakAbility, FActiveGameplayEffectHandle EffectHandle) const
{
	UAbilitySystemComponent* ASC = EffectHandle.GetOwningAbilitySystemComponent();
	const UGameplayAbility* InstigatorAbility = WeakAbility.Get();
	if (!ASC
	    || !InstigatorAbility
	    || EndedData.AbilityThatEnded != InstigatorAbility)
	{
		// Every other ability on same owner ends here too, only one that applied this effect takes it with it
		return;
	}

	ASC->RemoveActiveGameplayEffect(EffectHandle);
}
