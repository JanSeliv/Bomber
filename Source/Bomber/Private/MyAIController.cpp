// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAIController.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"

bool AMyAIController::UpdateAI_Implementation()
{
	AGeneratedMap* const LevelMap = USingletonLibrary::GetLevelMap();
	if (IS_VALID(LevelMap) == false			// The Level Map is not valid
		|| IS_VALID(MyCharacter) == false)  // The controller character is not valid
	{
		return false;
	}
	USingletonLibrary::PrintToLog(this, "----- UpdateAI -----", "Parent started");

#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld())  // [IsEditorNotPieWorld]
	{
		USingletonLibrary::PrintToLog(this, "[IsEditorNotPieWorld]UpdateAI", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(MyCharacter);
		AiMoveTo = FCell::ZeroCell;
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]

	return true;
}

void AMyAIController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IS_TRANSIENT(this))
	{
		return;
	}

	Possess(MyCharacter);
}

void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IS_VALID(InPawn) == false)
	{
		return;
	}

	MyCharacter = Cast<AMyCharacter>(InPawn);
}
