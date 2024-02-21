// Copyright (c) Yevhenii Selivanov

#include "NewAIUtils.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/NewAIBaseSubsystem.h"
#include "Subsystems/NewAIInGameSettingsSubsystem.h"
//---
#include "Engine/Engine.h"
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIUtils)

/*********************************************************************************************
 * Object getters
 ********************************************************************************************* */

// Returns New AI subsystem that provides access to the most important data like Data Asset
UNewAIBaseSubsystem* UNewAIUtils::GetBaseSubsystem(const UObject* OptionalWorldContext)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNewAIBaseSubsystem>() : nullptr;
}

// Returns New AI subsystem that handles In-Game Settings which are tweaked by player in Settings menu during the game
UNewAIInGameSettingsSubsystem* UNewAIUtils::GetInGameSettingsSubsystem(const UObject* OptionalWorldContext)
{
	return GEngine ? GEngine->GetEngineSubsystem<UNewAIInGameSettingsSubsystem>() : nullptr;
}
