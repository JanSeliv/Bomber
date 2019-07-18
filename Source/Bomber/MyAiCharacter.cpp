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
		UE_LOG_STR(this, "[PIE]UpdateAI", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(this);
		AiMoveTo = FCell();
	}
#endif  //WITH_EDITOR [PIE]
}
