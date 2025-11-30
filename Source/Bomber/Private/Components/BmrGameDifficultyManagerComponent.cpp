// Copyright (c) Yevhenii Selivanov

#include "Components/BmrGameDifficultyManagerComponent.h"

// Bomber
#include "Bomber.h"
#include "DataAssets/BmrModularGameFeatureSettings.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Structures/BmrGameDifficultyData.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Engine/Engine.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameDifficultyManagerComponent)

// Default constructor
UBmrGameDifficultyManagerComponent::UBmrGameDifficultyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}

// Returns this manager, is checked and wil crash if can't be obtained
UBmrGameDifficultyManagerComponent& UBmrGameDifficultyManagerComponent::Get()
{
	UBmrGameDifficultyManagerComponent* Manager = GetGameDifficultyManager();
	checkf(Manager, TEXT("ERROR: [%i] %hs:\n'Manager' is null!"), __LINE__, __FUNCTION__);
	return *Manager;
}

// Returns the pointer to this manager
UBmrGameDifficultyManagerComponent* UBmrGameDifficultyManagerComponent::GetGameDifficultyManager(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState(OptionalWorldContext);
	return MyGameState ? MyGameState->GetGameDifficultyManager() : nullptr;
}

// Returns current difficulty type, e.g: EBmrGameDifficulty::Easy
EBmrGameDifficulty UBmrGameDifficultyManagerComponent::GetDifficultyType() const
{
	// Map integer value (e.g EBmrGameDifficulty::Easy as 0) to the bit enum (e.g EBmrGameDifficulty::Easy as 1 << 0)
	return TO_ENUM(EBmrGameDifficulty, 1 << GetDifficultyLevel());
}

// Sets new game difficulty by enum type
void UBmrGameDifficultyManagerComponent::SetDifficultyType(EBmrGameDifficulty InDifficultyType)
{
	// Map bit enum (e.g EBmrGameDifficulty::Easy as 1 << 0) to the integer value (e.g EBmrGameDifficulty::Easy as 0)
	const int32 NewLevel = FMath::FloorLog2(TO_FLAG(InDifficultyType));
	SetDifficultyLevel(NewLevel);
}

// Returns true if the game difficulty level is matched with one or more specified types
bool UBmrGameDifficultyManagerComponent::HasDifficulty(int32 DifficultiesBitmask) const
{
	return EnumHasAnyFlags(GetDifficultyType(), TO_ENUM(EBmrGameDifficulty, DifficultiesBitmask));
}

// Set new difficulty level. Higher value bigger difficulty
void UBmrGameDifficultyManagerComponent::SetDifficultyLevel(int32 InLevel)
{
	if (ReplicatedDifficultyLevel == InLevel
	    || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Save to config (on host only)
	DifficultyLevel = InLevel;

	// Apply new difficulty level, so it will be replicated to clients
	ReplicatedDifficultyLevel = InLevel;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedDifficultyLevel, this);

	ApplyGameDifficulty();
}

// Applies the current difficulty by loading relevant features and unloading irrelevant ones
void UBmrGameDifficultyManagerComponent::UpdateGameFeaturesByDifficulty()
{
	const TArray<FBmrDifficultyGameFeaturesData>& DifficultyGameFeatures = UBmrModularGameFeatureSettings::Get().GetDifficultyGameFeatures();
	if (DifficultyGameFeatures.IsEmpty())
	{
		return;
	}

	TArray<FName> FeaturesToEnable, FeaturesToDisable;
	for (const FBmrDifficultyGameFeaturesData& It : DifficultyGameFeatures)
	{
		const EBmrGameDifficulty CurrentDifficulty = GetDifficultyType();
		const bool bShouldEnable = EnumHasAnyFlags(CurrentDifficulty, TO_ENUM(EBmrGameDifficulty, It.GameDifficulties));

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
void UBmrGameDifficultyManagerComponent::ApplyGameDifficulty()
{
	UpdateGameFeaturesByDifficulty();

	if (OnGameDifficultyChanged.IsBound())
	{
		OnGameDifficultyChanged.Broadcast(ReplicatedDifficultyLevel);
	}
}

// Called when the game difficulty level is changed
void UBmrGameDifficultyManagerComponent::OnRep_ReplicatedDifficultyLevel()
{
	ApplyGameDifficulty();
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when game starts or when spawned
void UBmrGameDifficultyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Apply difficulty level from config
	if (ensureMsgf(DifficultyLevel != INDEX_NONE, TEXT("ASSERT: [%i] %hs:\n'DifficultyLevel' was not loaded from config yet!"), __LINE__, __FUNCTION__))
	{
		SetDifficultyLevel(DifficultyLevel);
	}
}

// Returns properties that are replicated for the lifetime of the actor channel
void UBmrGameDifficultyManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedDifficultyLevel, Params);
}