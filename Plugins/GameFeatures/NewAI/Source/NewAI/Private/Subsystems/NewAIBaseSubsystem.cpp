// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NewAIBaseSubsystem.h"
//---
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "NewAIUtils.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIBaseSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNewAIBaseSubsystem& UNewAIBaseSubsystem::Get(const UObject* OptionalWorldContext)
{
	UNewAIBaseSubsystem* ThisSubsystem = UNewAIUtils::GetBaseSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'NewAIBaseSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

/*********************************************************************************************
 * Data Asset
 ********************************************************************************************* */

// Returns the NewAI data asset
const UNewAIDataAsset* UNewAIBaseSubsystem::GetNewAIDataAsset() const
{
	return NewAIDataAssetInternal.LoadSynchronous();
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the game starts
void UNewAIBaseSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	DisableOriginalAI();

	// There could be common logic for all AI agents line spawning NavMesh, EQS etc.
}

// Disables all vanilla AI agents to override its behavior by the NewAI feature
void UNewAIBaseSubsystem::DisableOriginalAI()
{
	static const FString AISetEnabledName = TEXT("Bomber.AI.SetEnabled");
	static IConsoleVariable* CVarAISetEnabled = IConsoleManager::Get().FindConsoleVariable(*AISetEnabledName);
	if (ensureMsgf(CVarAISetEnabled, TEXT("%s: 'CVarAISetEnabled' is not found, can not disable original AI"), *FString(__FUNCTION__)))
	{
		constexpr bool bSetEnabled = false;
		CVarAISetEnabled->Set(bSetEnabled);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the current game state was changed
void UNewAIBaseSubsystem::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	// ...
}
