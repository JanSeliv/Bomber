// Copyright (c) Yevhenii Selivanov

#include "NewAIManagerComponent.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#include "GeneratedMap.h"
#include "HAL/ConsoleManager.h"

// Sets default values for this component's properties
UNewAIManagerComponent::UNewAIManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns the NewAI data asset
const UNewAIDataAsset* UNewAIManagerComponent::GetNewAIDataAsset()
{
	const UNewAIManagerComponent* NewAIManager = AGeneratedMap::Get().FindComponentByClass<UNewAIManagerComponent>();
	return NewAIManager ? NewAIManager->NewAIDataAssetInternal : nullptr;
}

// Called when the game starts
void UNewAIManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	DisableOriginalAI();

	// There could be common logic for all AI agents line spawning NavMesh, EQS etc.
}

// Disables all vanilla AI agents to override its behavior by the NewAI feature
void UNewAIManagerComponent::DisableOriginalAI()
{
	static const FString AISetEnabledName = TEXT("Bomber.AI.SetEnabled");
	static IConsoleVariable* CVarAISetEnabled = IConsoleManager::Get().FindConsoleVariable(*AISetEnabledName);
	if (ensureMsgf(CVarAISetEnabled, TEXT("%s: 'CVarAISetEnabled' is not found, can not disable original AI"), *FString(__FUNCTION__)))
	{
		constexpr bool bSetEnabled = false;
		CVarAISetEnabled->Set(bSetEnabled);
	}
}

// Called when the current game state was changed
void UNewAIManagerComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	// ...
}
