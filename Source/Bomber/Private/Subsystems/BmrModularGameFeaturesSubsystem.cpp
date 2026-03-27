// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrModularGameFeaturesSubsystem.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "DataAssets/BmrModularGameFeatureSettings.h"
#include "MyUtilsLibraries/ModularGameFeaturePluginUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/BmrGeneratedMapSubsystem.h"

// UE
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

// Called when the Generated Map is initialized and ready, subscribes to ASC tag events for tag-driven features
void UBmrModularGameFeaturesSubsystem::OnGeneratedMapReady_Implementation(ABmrGeneratedMap* GeneratedMap)
{
	UAbilitySystemComponent* ASC = GeneratedMap ? GeneratedMap->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		return;
	}

	const FGameplayTagContainer& AllTags = UBmrModularGameFeatureSettings::Get().GetAllModularGameFeatureTags();
	for (const FGameplayTag& Tag : AllTags)
	{
		// Queue evaluation for next frame to batch multiple tag changes within the same frame
		ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddWeakLambda(this, [this](const FGameplayTag, int32)
		{
			if (EvaluationTimerHandle.IsValid())
			{
				// Is already queued
				return;
			}

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

// Is called when any of the tag-driven features tags is added or removed from the Generated Map ASC, evaluates all tag-driven features and loads/unloads them accordingly
void UBmrModularGameFeaturesSubsystem::OnModularGameFeatureTagChanged()
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	const UAbilitySystemComponent* ASC = GeneratedMap ? GeneratedMap->GetAbilitySystemComponent() : nullptr;
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

// Binds to GeneratedMap readiness to subscribe to ASC tag events
void UBmrModularGameFeaturesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UBmrGeneratedMapSubsystem* GeneratedMapSubsystem = UBmrGeneratedMapSubsystem::GetGeneratedMapSubsystem(this);
	if (!ensureMsgf(GeneratedMapSubsystem, TEXT("ERROR: [%i] %hs:\nGenerated Map Subsystem is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Subscribe when GeneratedMap becomes ready (ASC is initialized at that point)
	GeneratedMapSubsystem->OnGeneratedMapReady.AddDynamic(this, &ThisClass::OnGeneratedMapReady);
	if (GeneratedMapSubsystem->IsGeneratedMapReady())
	{
		OnGeneratedMapReady(GeneratedMapSubsystem->GetGeneratedMap());
	}
}

// Unloads tag-driven features on world end play so they don't leak across world boundaries
void UBmrModularGameFeaturesSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	Super::OnWorldEndPlay(InWorld);

	// Unload all tag-driven features
	TArray<FName> TagDrivenFeatures;
	UBmrModularGameFeatureSettings::Get().GetModularGameFeaturesByTags().GetKeys(TagDrivenFeatures);
	UModularGameFeaturePluginUtils::SetModularGameFeaturesActive(false, TagDrivenFeatures);
}