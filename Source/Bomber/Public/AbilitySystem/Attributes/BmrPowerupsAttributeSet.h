// Copyright (c) Yevhenii Selivanov

#pragma once

#include "AttributeSet.h"

// UE
#include "AbilitySystemComponent.h" // ATTRIBUTE_ACCESSORS_BASIC

#include "BmrPowerupsAttributeSet.generated.h"

struct FBmrPowerupTag;

/**
 * Attribute set for powerup-related attributes (powerups pick-up, character stats, etc.).
 */
UCLASS()
class BOMBER_API UBmrPowerupsAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** Returns the powerups attribute set for the specified owner. It will return nullptr if can't be obtained. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DefaultToSelf = "InOwner"))
	static const UBmrPowerupsAttributeSet* GetPowerupsAttributeSet(const UObject* InOwner);

	/** Returns the powerups attribute set for the specified owner. It will crash if can't be obtained. */
	static const UBmrPowerupsAttributeSet& Get(const UObject* InOwner);

	/** Converts powerup tag to base attribute */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "To Base Attribute (Powerup Tag)", CompactNodeTitle = "->", BlueprintAutocast))
	static FGameplayAttribute Conv_TagToBaseAttribute(FBmrPowerupTag InTag);

	/** Converts powerup tag to max attribute */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "To Max Attribute (Powerup Tag)", CompactNodeTitle = "->"))
	static FGameplayAttribute Conv_TagToMaxAttribute(FBmrPowerupTag InTag);

	/** Converts attribute to powerup tag */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "To Powerup Tag (Attribute)", CompactNodeTitle = "->", BlueprintAutocast))
	static FBmrPowerupTag Conv_AttributeToTag(const FGameplayAttribute& InAttribute);

	/** Returns the value of the attribute associated with the specified powerup tag, or -1 if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	float GetPowerupValueByTag(FBmrPowerupTag InTag) const;

	/** Returns the value of the max attribute associated with the specified powerup tag, or -1 if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	float GetPowerupMaxValueByTag(FBmrPowerupTag InTag) const;

	/** Returns true if the current value of the powerup associated with the specified tag is at its maximum. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsPowerupValueAtMax(FBmrPowerupTag InTag) const;

	/*********************************************************************************************
	 * Data attributes
	 ********************************************************************************************* */
public:
	/** Current explosion radius enhancement from fire powerups. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Powerup_Fire", Category = "[Bomber]")
	FGameplayAttributeData Powerup_Fire;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Powerup_Fire)

	/** Maximum allowed explosion radius. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Powerup_MaxFire", Category = "[Bomber]")
	FGameplayAttributeData Powerup_MaxFire;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Powerup_MaxFire)

	/** Amount of skate powerups collected (each adds +100 speed to base movement speed). */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Powerup_Skate", Category = "[Bomber]")
	FGameplayAttributeData Powerup_Skate;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Powerup_Skate)

	/** Maximum allowed skate powerups. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Powerup_MaxSkate", Category = "[Bomber]")
	FGameplayAttributeData Powerup_MaxSkate;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Powerup_MaxSkate)

	/** Current available bombs for placement: decremented when placed, incremented when exploded. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Powerup_BombsAvailable", Category = "[Bomber]")
	FGameplayAttributeData Powerup_BombsAvailable;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Powerup_BombsAvailable)

	/** Maximum bomb capacity. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_Powerup_MaxBombs", Category = "[Bomber]")
	FGameplayAttributeData Powerup_MaxBombs;
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Powerup_MaxBombs)

	/*********************************************************************************************
	 * OnRep notifies
	 ********************************************************************************************* */
protected:
	UFUNCTION()
	void OnRep_Powerup_Fire(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Powerup_MaxFire(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Powerup_Skate(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Powerup_MaxSkate(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Powerup_BombsAvailable(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Powerup_MaxBombs(const FGameplayAttributeData& OldValue) const;

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
};