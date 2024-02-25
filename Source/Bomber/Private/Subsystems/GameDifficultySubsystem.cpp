// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GameDifficultySubsystem.h"
//---
#include "Bomber.h"
//---
#include "Engine/Engine.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameDifficultySubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UGameDifficultySubsystem& UGameDifficultySubsystem::Get()
{
	UGameDifficultySubsystem* Subsystem = GetGameDifficultySubsystem();
	checkf(Subsystem, TEXT("%s: 'Subsystem' is null"), *FString(__FUNCTION__));
	return *Subsystem;
}

// Returns the pointer to this Subsystem
UGameDifficultySubsystem* UGameDifficultySubsystem::GetGameDifficultySubsystem(const UObject* OptionalWorldContext/* = nullptr*/)
{
	return GEngine ? GEngine->GetEngineSubsystem<UGameDifficultySubsystem>() : nullptr;
}

// Returns current difficulty type, e.g: EGameDifficulty::Easy
EGameDifficulty UGameDifficultySubsystem::GetDifficultyType() const
{
	// Map integer value (e.g EGameDifficulty::Easy as 0) to the bit enum (e.g EGameDifficulty::Easy as 1 << 0)
	return TO_ENUM(EGameDifficulty, 1 << DifficultyLevelInternal);
}

// Sets new game difficulty by enum type
void UGameDifficultySubsystem::SetDifficultyType(EGameDifficulty InDifficultyType)
{
	// Map bit enum (e.g EGameDifficulty::Easy as 1 << 0) to the integer value (e.g EGameDifficulty::Easy as 0)
	const int32 NewLevel = FMath::FloorLog2(TO_FLAG(InDifficultyType));
	SetDifficultyLevel(NewLevel);
}

// Set new difficulty level. Higher value bigger difficulty
void UGameDifficultySubsystem::SetDifficultyLevel(int32 InLevel)
{
	if (DifficultyLevelInternal == InLevel)
	{
		return;
	}

	DifficultyLevelInternal = InLevel;

	ApplyGameDifficulty();
}

// Applies current difficulty level to the game
void UGameDifficultySubsystem::ApplyGameDifficulty()
{
	if (OnGameDifficultyChanged.IsBound())
	{
		OnGameDifficultyChanged.Broadcast(DifficultyLevelInternal);
	}
}
