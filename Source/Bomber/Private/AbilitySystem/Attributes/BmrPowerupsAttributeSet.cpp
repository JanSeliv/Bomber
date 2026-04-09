// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"

// UE
#include "AbilitySystemGlobals.h"
#include "DataRegistries/BmrPowerupRow.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Structures/BmrPowerupTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupsAttributeSet)

// Returns the powerups attribute set for the specified owner. It will return nullptr if can't be obtained
const UBmrPowerupsAttributeSet* UBmrPowerupsAttributeSet::GetPowerupsAttributeSet(const UObject* InOwner)
{
	const UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(InOwner);
	if (!ASC)
	{
		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<AActor>(InOwner));
	}
	const UAttributeSet* AttributeSet = ASC ? ASC->GetAttributeSet(StaticClass()) : nullptr;
	return Cast<UBmrPowerupsAttributeSet>(AttributeSet);
}

// Returns the powerups attribute set for the specified owner. It will crash if can't be obtained
const UBmrPowerupsAttributeSet& UBmrPowerupsAttributeSet::Get(const UObject* InOwner)
{
	const UBmrPowerupsAttributeSet* PowerupsAttributeSet = GetPowerupsAttributeSet(InOwner);
	checkf(PowerupsAttributeSet, TEXT("ERROR: [%i] %hs:\n'PowerupsAttributeSet' is null!"), __LINE__, __FUNCTION__);
	return *PowerupsAttributeSet;
}

// Returns the powerup attribute associated with the given tag, or an empty attribute if not found
FGameplayAttribute UBmrPowerupsAttributeSet::Conv_TagToBaseAttribute(FBmrPowerupTag InTag)
{
	if (!InTag.IsValid())
	{
		return FGameplayAttribute();
	}

	if (InTag == FBmrPowerupTag::Fire)
	{
		return GetPowerup_FireAttribute();
	}

	if (InTag == FBmrPowerupTag::Skate)
	{
		return GetPowerup_SkateAttribute();
	}

	if (InTag == FBmrPowerupTag::Bomb)
	{
		return GetPowerup_BombsAvailableAttribute();
	}

	ensureMsgf(false, TEXT("ASSERT: [%i] %hs:\n'%s' powerup tag is not recognized!"), __LINE__, __FUNCTION__, *InTag.ToString());
	return FGameplayAttribute();
}

// Returns the max powerup attribute associated with the given tag, or an empty attribute if not found
FGameplayAttribute UBmrPowerupsAttributeSet::Conv_TagToMaxAttribute(FBmrPowerupTag InTag)
{
	if (!InTag.IsValid())
	{
		return FGameplayAttribute();
	}

	if (InTag == FBmrPowerupTag::Fire)
	{
		return GetPowerup_MaxFireAttribute();
	}

	if (InTag == FBmrPowerupTag::Skate)
	{
		return GetPowerup_MaxSkateAttribute();
	}

	if (InTag == FBmrPowerupTag::Bomb)
	{
		return GetPowerup_MaxBombsAttribute();
	}

	ensureMsgf(false, TEXT("ASSERT: [%i] %hs:\n'%s' powerup tag is not recognized!"), __LINE__, __FUNCTION__, *InTag.ToString());
	return FGameplayAttribute();
}

// Returns the powerup tag associated with the specified attribute, or an invalid tag if not found
FBmrPowerupTag UBmrPowerupsAttributeSet::Conv_AttributeToTag(const FGameplayAttribute& InAttribute)
{
	if (!InAttribute.IsValid())
	{
		return FBmrPowerupTag::None;
	}

	if (InAttribute == GetPowerup_FireAttribute()
	    || InAttribute == GetPowerup_MaxFireAttribute())
	{
		return FBmrPowerupTag::Fire;
	}

	if (InAttribute == GetPowerup_SkateAttribute()
	    || InAttribute == GetPowerup_MaxSkateAttribute())
	{
		return FBmrPowerupTag::Skate;
	}

	if (InAttribute == GetPowerup_BombsAvailableAttribute()
	    || InAttribute == GetPowerup_MaxBombsAttribute())
	{
		return FBmrPowerupTag::Bomb;
	}

	ensureMsgf(false, TEXT("ASSERT: [%i] %hs:\n'%s' attribute is not recognized!"), __LINE__, __FUNCTION__, *InAttribute.GetName());
	return FBmrPowerupTag::None;
}

// Returns the value of the powerup attribute associated with the given tag, or -1 if not found
float UBmrPowerupsAttributeSet::GetPowerupValueByTag(FBmrPowerupTag InTag) const
{
	const FGameplayAttribute PowerupAttribute = Conv_TagToBaseAttribute(InTag);
	constexpr float InvalidPowerupValue = -1.f;
	return PowerupAttribute.IsValid() ? PowerupAttribute.GetNumericValue(this) : InvalidPowerupValue;
}

// Returns the value of the max powerup attribute associated with the given tag, or -1 if not found
float UBmrPowerupsAttributeSet::GetPowerupMaxValueByTag(FBmrPowerupTag InTag) const
{
	const FGameplayAttribute PowerupMaxAttribute = Conv_TagToMaxAttribute(InTag);
	constexpr float InvalidPowerupValue = -1.f;
	return PowerupMaxAttribute.IsValid() ? PowerupMaxAttribute.GetNumericValue(this) : InvalidPowerupValue;
}

// Returns true if the current value of the powerup associated with the given tag is at its maximum
bool UBmrPowerupsAttributeSet::IsPowerupValueAtMax(FBmrPowerupTag InTag) const
{
	return GetPowerupValueByTag(InTag) >= GetPowerupMaxValueByTag(InTag);
}

/*********************************************************************************************
 * OnRep notifies
 ********************************************************************************************* */

void UBmrPowerupsAttributeSet::OnRep_Powerup_Fire(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Powerup_Fire, OldValue);
}

void UBmrPowerupsAttributeSet::OnRep_Powerup_MaxFire(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Powerup_MaxFire, OldValue);
}

void UBmrPowerupsAttributeSet::OnRep_Powerup_Skate(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Powerup_Skate, OldValue);
}

void UBmrPowerupsAttributeSet::OnRep_Powerup_MaxSkate(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Powerup_MaxSkate, OldValue);
}

void UBmrPowerupsAttributeSet::OnRep_Powerup_BombsAvailable(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Powerup_BombsAvailable, OldValue);
}

void UBmrPowerupsAttributeSet::OnRep_Powerup_MaxBombs(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Powerup_MaxBombs, OldValue);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel
void UBmrPowerupsAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Powerup_Fire, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Powerup_MaxFire, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Powerup_Skate, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Powerup_MaxSkate, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Powerup_BombsAvailable, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Powerup_MaxBombs, Params);
}

// Called just before any modification happens to an attribute
void UBmrPowerupsAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	PreAttributeChangeClamped(Attribute, NewValue);
}

void UBmrPowerupsAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	PreAttributeChangeClamped(Attribute, NewValue);
}

void UBmrPowerupsAttributeSet::PreAttributeChangeClamped(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// E.g: if base attribute became larger than max, then clamp the base attribute to its max
	const FBmrPowerupTag MaxPowerup = Conv_AttributeToTag(Attribute);
	const FGameplayAttribute MaxAttribute = Conv_TagToMaxAttribute(MaxPowerup);
	const bool bIsBaseAttribute = Attribute != MaxAttribute;
	if (bIsBaseAttribute)
	{
		constexpr float MinValue = 0.f;
		const float MaxValue = MaxAttribute.GetNumericValue(this);
		NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
	}
}

// Is overridden to reclamp after changing dynamic max attributes
void UBmrPowerupsAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'ASC' is null!"), __LINE__, __FUNCTION__);

	const FBmrPowerupTag PowerupTag = Conv_AttributeToTag(Attribute);
	const FGameplayAttribute BaseAttribute = Conv_TagToBaseAttribute(PowerupTag);
	const float BaseValue = BaseAttribute.GetNumericValue(this);

	// Clamp the base attribute to new max when max attribute was dynamically decreased
	const bool bIsMaxAttributeChanged = Attribute != BaseAttribute;
	if (bIsMaxAttributeChanged
	    && BaseValue > NewValue)
	{
		ASC->ApplyModToAttribute(BaseAttribute, EGameplayModOp::Override, NewValue);
	}

	// Apply max collect gameplay effect if applicable
	const FBmrPowerupRow* PowerupRow = FBmrPowerupRow::GetRowByPowerupTag(PowerupTag);
	const TSubclassOf<UGameplayEffect> MaxCollectGameplayEffect = PowerupRow ? PowerupRow->MaxCollectGameplayEffect.Get() : nullptr;
	if (MaxCollectGameplayEffect)
	{
		const UGameplayEffect* EffectCDO = MaxCollectGameplayEffect.GetDefaultObject();
		const FGameplayTagContainer& PowerupTags = EffectCDO ? EffectCDO->GetGrantedTags() : FGameplayTagContainer::EmptyContainer;
		ensureMsgf(PowerupTags.IsValid(), TEXT("ERROR: [%i] %hs:\nMaxCollectGameplayEffect has no granted tags for powerup '%s'"), __LINE__, __FUNCTION__, *PowerupTag.ToString());
		const bool bIsEffectCurrentlyActive = ASC->HasAllMatchingGameplayTags(PowerupTags);
		if (bIsEffectCurrentlyActive != IsPowerupValueAtMax(PowerupTag))
		{
			if (bIsEffectCurrentlyActive)
			{
				ASC->RemoveActiveEffectsWithGrantedTags(PowerupTags);
			}
			else
			{
				ASC->ApplyGameplayEffectToSelf(EffectCDO, /*Level*/ 1.f, ASC->MakeEffectContext());
			}
		}
	}
}