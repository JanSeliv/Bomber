// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NewAIInGameSettingsSubsystem.h"
//---
#include "NewAIUtils.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIInGameSettingsSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNewAIInGameSettingsSubsystem& UNewAIInGameSettingsSubsystem::Get()
{
	UNewAIInGameSettingsSubsystem* Subsystem = UNewAIUtils::GetInGameSettingsSubsystem();
	checkf(Subsystem, TEXT("%s: 'NewAIInGameSettingsSubsystem' is null"), *FString(__FUNCTION__));
	return *Subsystem;
}

// Set new difficulty level. Higher value bigger difficulty
void UNewAIInGameSettingsSubsystem::SetDifficultyLevel(int32 InLevel)
{
	if (DifficultyLevelInternal == InLevel)
	{
		return;
	}

	DifficultyLevelInternal = InLevel;

	OnNewAIDifficultyChanged.Broadcast(DifficultyLevelInternal);
}
