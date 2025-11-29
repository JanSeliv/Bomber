// Copyright (c) Yevhenii Selivanov

#include "Components/GameDifficultyManagerComponent.h"

// Bomber
#include "Bomber.h"
#include "DataAssets/ModularGameFeatureSettings.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Structures/GameDifficultyData.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

// UE
#include "Engine/Engine.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameDifficultyManagerComponent)

// Default constructor
UGameDifficultyManagerComponent::UGameDifficultyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}

// Returns this manager, is checked and wil crash if can't be obtained
UGameDifficultyManagerComponent& UGameDifficultyManagerComponent::Get()
{
	UGameDifficultyManagerComponent* Manager = GetGameDifficultyManager();
	checkf(Manager, TEXT("ERROR: [%i] %hs:\n'Manager' is null!"), __LINE__, __FUNCTION__);
	return *Manager;
}

// Returns the pointer to this manager
UGameDifficultyManagerComponent* UGameDifficultyManagerComponent::GetGameDifficultyManager(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState(OptionalWorldContext);
	return MyGameState ? MyGameState->GetGameDifficultyManager() : nullptr;
}

// Returns current difficulty type, e.g: EGameDifficulty::Easy
EGameDifficulty UGameDifficultyManagerComponent::GetDifficultyType() const
{
	// Map integer value (e.g EGameDifficulty::Easy as 0) to the bit enum (e.g EGameDifficulty::Easy as 1 << 0)
	return TO_ENUM(EGameDifficulty, 1 << GetDifficultyLevel());
}

// Sets new game difficulty by enum type
void UGameDifficultyManagerComponent::SetDifficultyType(EGameDifficulty InDifficultyType)
{
	// Map bit enum (e.g EGameDifficulty::Easy as 1 << 0) to the integer value (e.g EGameDifficulty::Easy as 0)
	const int32 NewLevel = FMath::FloorLog2(TO_FLAG(InDifficultyType));
	SetDifficultyLevel(NewLevel);
}

// Returns true if the game difficulty level is matched with one or more specified types
bool UGameDifficultyManagerComponent::HasDifficulty(int32 DifficultiesBitmask) const
{
	return EnumHasAnyFlags(GetDifficultyType(), TO_ENUM(EGameDifficulty, DifficultiesBitmask));
}

// Set new difficulty level. Higher value bigger difficulty
void UGameDifficultyManagerComponent::SetDifficultyLevel(int32 InLevel)
{
	if (ReplicatedDifficultyLevelInternal == InLevel
	    || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Save to config (on host only)
	DifficultyLevelInternal = InLevel;

	// Apply new difficulty level, so it will be replicated to clients
	ReplicatedDifficultyLevelInternal = InLevel;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedDifficultyLevelInternal, this);

	ApplyGameDifficulty();
}

// Applies the current difficulty by loading relevant features and unloading irrelevant ones
void UGameDifficultyManagerComponent::UpdateGameFeaturesByDifficulty()
{
	const TArray<FDifficultyGameFeaturesData>& DifficultyGameFeatures = UModularGameFeatureSettings::Get().GetDifficultyGameFeatures();
	if (DifficultyGameFeatures.IsEmpty())
	{
		return;
	}

	TArray<FName> FeaturesToEnable, FeaturesToDisable;
	for (const FDifficultyGameFeaturesData& It : DifficultyGameFeatures)
	{
		const EGameDifficulty CurrentDifficulty = GetDifficultyType();
		const bool bShouldEnable = EnumHasAnyFlags(CurrentDifficulty, TO_ENUM(EGameDifficulty, It.GameDifficulties));

		if (bShouldEnable)
		{
			FeaturesToEnable.AddUnique(It.ModularGameFeatureName);
		}
		else
		{
			FeaturesToDisable.AddUnique(It.ModularGameFeatureName);
		}
	}

	// First disable irrelevant features
	UGameplayUtilsLibrary::SetGameFeaturesEnabled(false, FeaturesToDisable);

	// Then enable relevant features
	UGameplayUtilsLibrary::SetGameFeaturesEnabled(true, FeaturesToEnable);
}

// Applies current difficulty level to the game
void UGameDifficultyManagerComponent::ApplyGameDifficulty()
{
	UpdateGameFeaturesByDifficulty();

	if (OnGameDifficultyChanged.IsBound())
	{
		OnGameDifficultyChanged.Broadcast(ReplicatedDifficultyLevelInternal);
	}
}

// Called when the game difficulty level is changed
void UGameDifficultyManagerComponent::OnRep_ReplicatedDifficultyLevel()
{
	ApplyGameDifficulty();
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when game starts or when spawned
void UGameDifficultyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Apply difficulty level from config
	if (ensureMsgf(DifficultyLevelInternal != INDEX_NONE, TEXT("ASSERT: [%i] %hs:\n'DifficultyLevelInternal' was not loaded from config yet!"), __LINE__, __FUNCTION__))
	{
		SetDifficultyLevel(DifficultyLevelInternal);
	}
}

// Returns properties that are replicated for the lifetime of the actor channel
void UGameDifficultyManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedDifficultyLevelInternal, Params);
}