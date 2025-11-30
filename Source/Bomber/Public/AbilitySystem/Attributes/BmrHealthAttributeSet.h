// Copyright (c) Yevhenii Selivanov

#pragma once

#include "AttributeSet.h"

// UE
#include "AbilitySystemComponent.h" // ATTRIBUTE_ACCESSORS_BASIC

#include "BmrHealthAttributeSet.generated.h"

/**
 * Attribute set for health and damage-related attributes.
 */
UCLASS()
class BOMBER_API UBmrHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** Returns the health attribute set for the specified owner. It will return nullptr if can't be obtained. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DefaultToSelf = "InOwner"))
	static const UBmrHealthAttributeSet* GetHealthAttributeSet(const UObject* InOwner);

	/** Returns the health attribute set for the specified owner. It will crash if can't be obtained. */
	static const UBmrHealthAttributeSet& Get(const UObject* InOwner);

	/*********************************************************************************************
	 * Data attributes
	 ********************************************************************************************* */
public:
	/** Current player health (when 0 triggers death ability). */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Health", Category = "[Bomber]")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Health)

	/** Maximum health value for clamping. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_MaxHealth", Category = "[Bomber]")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, MaxHealth)

	/** OutcomingDamage can be changed via gameplay effect modifiers (powerups, buffs, debuffs).
	 * Flow: Multiple effects modify this value → Damage execution captures final result → Applies to target IncomingDamage.
	 * Replicated for UI display and feedback purposes. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_OutcomingDamage", Category = "[Bomber]")
	FGameplayAttributeData OutcomingDamage;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, OutcomingDamage)

	/** Meta attribute for incoming damage calculation. Set only by damage execution calculations, never by direct modifiers.
	 * Flow: Damage execution captures source OutcomingDamage → Sets this value → PostGameplayEffectExecute processes it into health reduction.
	 * Non-replicated as it's immediately processed and cleared. Hidden from modifiers to prevent direct manipulation. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Bomber]", meta = (HideFromModifiers = "true"))
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, IncomingDamage)

protected:
	/** Used to track when the health reaches 0. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bOutOfHealth = false;

	/*********************************************************************************************
	 * OnRep notifies
	 ********************************************************************************************* */
protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_OutcomingDamage(const FGameplayAttributeData& OldValue) const;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called just before any modification happens to an attribute. */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	void PreAttributeChangeClamped(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** Is overridden to reclamp after changing dynamic max attributes. */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/** Is overridden to prevent any damage in certain cases. */
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;

	/** Is overridden to handle damage processing and trigger death events. */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};