// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Attributes/BmrHealthAttributeSet.h"

// UE
#include "AbilitySystemGlobals.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Structures/BmrGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrHealthAttributeSet)

// Returns the health attribute set for the specified owner. It will return nullptr if can't be obtained
const UBmrHealthAttributeSet* UBmrHealthAttributeSet::GetHealthAttributeSet(const UObject* InOwner)
{
	const UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(InOwner);
	if (!ASC)
	{
		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<AActor>(InOwner));
	}
	const UAttributeSet* AttributeSet = ASC ? ASC->GetAttributeSet(StaticClass()) : nullptr;
	return Cast<UBmrHealthAttributeSet>(AttributeSet);
}

// Returns the health attribute set for the specified owner. It will crash if can't be obtained
const UBmrHealthAttributeSet& UBmrHealthAttributeSet::Get(const UObject* InOwner)
{
	const UBmrHealthAttributeSet* HealthAttributeSet = GetHealthAttributeSet(InOwner);
	checkf(HealthAttributeSet, TEXT("ERROR: [%i] %hs:\n'HealthAttributeSet' is null!"), __LINE__, __FUNCTION__);
	return *HealthAttributeSet;
}

/*********************************************************************************************
 * OnRep notifies
 ********************************************************************************************* */

void UBmrHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue);
}

void UBmrHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldValue);
}

void UBmrHealthAttributeSet::OnRep_OutcomingDamage(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, OutcomingDamage, OldValue);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel
void UBmrHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Health, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MaxHealth, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OutcomingDamage, Params);
}

// Called just before any modification happens to an attribute
void UBmrHealthAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	PreAttributeChangeClamped(Attribute, NewValue);
}

void UBmrHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	PreAttributeChangeClamped(Attribute, NewValue);
}

void UBmrHealthAttributeSet::PreAttributeChangeClamped(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

// Is overridden to reclamp after changing dynamic max attributes
void UBmrHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health
		if (GetHealth() > NewValue)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			checkf(ASC, TEXT("ERROR: [%i] %hs:\n'ASC' is null!"), __LINE__, __FUNCTION__);
			ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && GetHealth() > 0.f)
	{
		bOutOfHealth = false;
	}
}

// Is overridden to prevent any damage in certain cases
bool UBmrHealthAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute()
	    && Data.Target.HasMatchingGameplayTag(BmrGameplayTags::GameplayEffect::Block::IncomingDamage))
	{
		// Prevent damage when immunity is active
		Data.EvaluatedData.Magnitude = 0.f;
		return false;
	}

	return true;
}

// Is overridden to handle damage processing and trigger death events
void UBmrHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	constexpr float MinimumHealth = 0.f;

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// Convert into -Health and then clamp
		SetHealth(FMath::Clamp(GetHealth() - GetIncomingDamage(), MinimumHealth, GetMaxHealth()));
		SetIncomingDamage(0.f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp and fall into out of health handling below
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
	}

	if (GetHealth() <= 0.f && !bOutOfHealth)
	{
		FGameplayEventData EventData;
		EventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
		Data.Target.HandleGameplayEvent(BmrGameplayTags::Event::Player_Death, &EventData);
	}

	// Check health again in case an event above changed it.
	bOutOfHealth = GetHealth() <= 0.f;
}