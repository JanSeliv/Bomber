// Copyright (c) Yevhenii Selivanov

#include "Components/NewAIAgentComponent.h"
//---
#include "Data/NewAIDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyUtilsLibraries/AIUtilsLibrary.h"
#include "Subsystems/NewAIInGameSettingsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIAgentComponent)

// Sets default values for this component's properties
UNewAIAgentComponent::UNewAIAgentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns AI Controller of this component
AAIController* UNewAIAgentComponent::GetAIController() const
{
	return Cast<AAIController>(GetOwner());
}

AAIController& UNewAIAgentComponent::GetAIControllerChecked() const
{
	AAIController* AIController = GetAIController();
	checkf(AIController, TEXT("ERROR: [%i] %s:\n'AIController' is null!"), __LINE__, *FString(__FUNCTION__));
	return *AIController;
}

// Called when the game starts
void UNewAIAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	UNewAIInGameSettingsSubsystem::Get().OnNewAIDifficultyChanged.AddDynamic(this, &ThisClass::OnNewAIDifficultyChanged);
}

// Starts or stops the behavior tree
void UNewAIAgentComponent::HandleBehaviorTree()
{
	constexpr int32 LegacyLevel = 3;
	const bool bWantsRunBT = UNewAIInGameSettingsSubsystem::Get().GetDifficultyLevel() != LegacyLevel
		&& AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::InGame;

	AAIController& AIController = GetAIControllerChecked();
	UBehaviorTree* BehaviorTree = UNewAIDataAsset::Get().GetBehaviorTree();
	if (bWantsRunBT == UAIUtilsLibrary::IsRunningAnyBehaviorTree(&AIController)
		|| !ensureMsgf(BehaviorTree, TEXT("%s:'BehaviorTree' is not valid"), *FString(__FUNCTION__)))
	{
		// Is already in the desired state
		return;
	}

	if (bWantsRunBT)
	{
		AIController.RunBehaviorTree(BehaviorTree);
	}
	else
	{
		const FString Reason = FString::Printf(TEXT("NewAIAgentComponent: %s"), *BehaviorTree->GetName());
		UBrainComponent* BrainComp = AIController.GetBrainComponent();
		checkf(BrainComp, TEXT("ERROR: [%i] %s:\n'BrainComp' is null!"), __LINE__, *FString(__FUNCTION__));
		BrainComp->StopLogic(Reason);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the current game state was changed
void UNewAIAgentComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	HandleBehaviorTree();
}

// Called when new difficulty level is set
void UNewAIAgentComponent::OnNewAIDifficultyChanged_Implementation(int32 NewDifficultyLevel)
{
	HandleBehaviorTree();
}
