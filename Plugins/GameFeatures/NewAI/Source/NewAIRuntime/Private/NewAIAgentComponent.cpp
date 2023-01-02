// Copyright (c) Yevhenii Selivanov

#include "NewAIAgentComponent.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#include "AIController.h"
#include "NewAIDataAsset.h"
#include "NewAIManagerComponent.h"

// Sets default values for this component's properties
UNewAIAgentComponent::UNewAIAgentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Called when the game starts
void UNewAIAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	AIControllerInternal = Cast<AAIController>(GetOwner());
	checkf(AIControllerInternal, TEXT("CRITICAL ERROR: %s:'AIControllerInternal' is not valid"), *FString(__FUNCTION__));
	// There you might want to Run BT AIControllerInternal->RunBehaviorTree(...) for this agent etc.
}

// Called when the current game state was changed
void UNewAIAgentComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	// ...
	if (CurrentGameState == ECurrentGameState::InGame)
	{
		UBehaviorTree* BehaviorTree = UNewAIManagerComponent::GetNewAIDataAsset()->GetBehaviorTree();
		if (ensureMsgf(BehaviorTree, TEXT("%s:'BehaviorTree' is not valid"), *FString(__FUNCTION__)))
		{
			AIControllerInternal->RunBehaviorTree(BehaviorTree);
		}
	}
}
