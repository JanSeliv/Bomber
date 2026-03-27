// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrModularGameFeaturesSubsystem.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "DataAssets/BmrModularGameFeatureSettings.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "Subsystems/BmrGeneratedMapSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrModularGameFeaturesSubsystem)

// Returns this subsystem, is checked and will crash if can't be obtained
UBmrModularGameFeaturesSubsystem& UBmrModularGameFeaturesSubsystem::Get(const UObject* WorldContextObject /* = nullptr*/)
{
	UBmrModularGameFeaturesSubsystem* Subsystem = GetModularGameFeaturesSubsystem(WorldContextObject);
	checkf(Subsystem, TEXT("ERROR: [%i] %hs:\n'Subsystem' is null!"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this subsystem
UBmrModularGameFeaturesSubsystem* UBmrModularGameFeaturesSubsystem::GetModularGameFeaturesSubsystem(const UObject* WorldContextObject /* = nullptr*/)
{
	const UWorld* FoundWorld = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	return FoundWorld ? FoundWorld->GetSubsystem<UBmrModularGameFeaturesSubsystem>() : nullptr;
}

/*********************************************************************************************
 * Tag-Driven Features
 ********************************************************************************************* */

// Called when world data assets are loaded, subscribes to ASC tag events for tag-driven features
void UBmrModularGameFeaturesSubsystem::OnGeneratedMapReady_Implementation(const FGameplayEventData& Payload)
{
	UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Listen to tag changes on ASC for tags that drive modular game features
	const FGameplayTagContainer& AllFeatureTags = UBmrModularGameFeatureSettings::Get().GetAllModularGameFeatureTags();
	for (const FGameplayTag& Tag : AllFeatureTags)
	{
		ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddWeakLambda(this, [this](const FGameplayTag, int32)
		{
			if (EvaluationTimerHandle.IsValid())
			{
				// Is already queued
				return;
			}

			// Queue evaluation for next frame to batch multiple tag changes within the same frame
			EvaluationTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
			{
				EvaluationTimerHandle.Invalidate();
				OnModularGameFeatureTagChanged();
			}));
		});
	}

	// Immediate evaluation for tags already present on ASC
	OnModularGameFeatureTagChanged();
}

// Is called when any of the tag-driven features tags is added or removed from the world ASC, evaluates all tag-driven features and loads/unloads them accordingly
void UBmrModularGameFeaturesSubsystem::OnModularGameFeatureTagChanged()
{
	const UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("ERROR: [%i] %hs:\nASC is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const TMap<FName, FGameplayTagContainer>& FeaturesByTags = UBmrModularGameFeatureSettings::Get().GetModularGameFeaturesByTags();
	if (FeaturesByTags.IsEmpty())
	{
		// No tag-driven features, nothing to evaluate
		return;
	}

	TArray<FName> FeaturesToEnable;
	TArray<FName> FeaturesToDisable;
	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	for (const TTuple<FName, FGameplayTagContainer>& Pair : FeaturesByTags)
	{
		const bool bShouldEnable = OwnedTags.HasAny(Pair.Value);
		const bool bIsActive = UModularGameFeaturePluginUtils::IsModularGameFeatureActive(Pair.Key);
		if (bShouldEnable == bIsActive)
		{
			// Already in expected state
			continue;
		}

		if (bShouldEnable)
		{
			FeaturesToEnable.AddUnique(Pair.Key);
		}
		else
		{
			FeaturesToDisable.AddUnique(Pair.Key);
		}
	}

	if (FeaturesToDisable.IsEmpty()
	    && FeaturesToEnable.IsEmpty())
	{
		// Nothing to change
		return;
	}

	// First disable irrelevant features, then enable relevant ones
	UModularGameFeaturePluginUtils::SetModularGameFeaturesActive(false, FeaturesToDisable);
	UModularGameFeaturePluginUtils::SetModularGameFeaturesActive(true, FeaturesToEnable);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Subscribes to GeneratedMap_Ready for tag-driven features
void UBmrModularGameFeaturesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IS_TRANSIENT(this))
	{
		return;
	}

	BIND_ON_GENERATED_MAP_READY(this, UBmrModularGameFeaturesSubsystem::OnGeneratedMapReady);
}

// Unloads all managed features on world end play
void UBmrModularGameFeaturesSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	Super::OnWorldEndPlay(InWorld);

	if (IS_TRANSIENT(this))
	{
		return;
	}

	// Unload tag-driven features so they don't leak across world boundaries
	TArray<FName> TagDrivenFeatures;
	UBmrModularGameFeatureSettings::Get().GetModularGameFeaturesByTags().GetKeys(TagDrivenFeatures);
	UModularGameFeaturePluginUtils::SetModularGameFeaturesActive(false, TagDrivenFeatures);
}