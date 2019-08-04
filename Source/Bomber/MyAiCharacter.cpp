// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAiCharacter.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "SingletonLibrary.h"

void AMyAiCharacter::UpdateAI_Implementation()
{
	AGeneratedMap* const LevelMap = USingletonLibrary::GetLevelMap();
	if (IS_VALID(LevelMap) == false)  // The Level Map is not valid
	{
		return;
	}

#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true)  // for editor only
	{
		USingletonLibrary::PrintToLog(this, "[PIE]UpdateAI", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(this);
		AiMoveTo = FCell::ZeroCell;
	}
#endif  //WITH_EDITOR [PIE]
}
