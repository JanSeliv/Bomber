// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAiCharacter.h"

#include "Bomber.h"
#include "SingletonLibrary.h"

void AMyAiCharacter::UpdateAI_Implementation()
{
	AGeneratedMap* const LevelMap = USingletonLibrary::GetLevelMap(GetWorld());
	if (LevelMap == nullptr)  // Level Map is null
	{
		return;
	}

#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true)  // for editor only
	{
		UE_LOG_STR(this, "[PIE]UpdateAI", "Answered");
		USingletonLibrary::ClearOwnerTextRenders(this);
		AiMoveTo = FCell();
	}
#endif
}

void AMyAiCharacter::OnConstruction(const FTransform& Transform)
{
	if (IS_TRANSIENT(this) == true)
	{
		return;
	}

	// Call the AMyCharacter parent
	Super::OnConstruction(Transform);
}
