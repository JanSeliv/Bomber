// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAIController.h"

#include "Kismet/KismetMathLibrary.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"

bool AMyAIController::UpdateAI_Implementation(
	FCell& F0,
	TSet<FCell>& Free,
	bool& bIsDangerous)
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
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

	// ----- Part 0: Before iterations -----

	// Set the START cell searching bot location
	//const FCell F0(MyCharacter);

	// Searching 'SAFE NEIGHBORS'
	// TSet<FCell> Free;
	// bool bIsDangerous
	for (int32 i = 0; i < 2; ++i)  // two searches (safe and free)
	{
		bIsDangerous = static_cast<bool>(i);
		Free = LevelMap->GetSidesCells(F0, bIsDangerous ? EPathTypesEnum::Free : EPathTypesEnum::Safe);
		if (!bIsDangerous && Free.Num() > 0)
		{
			break;
		}
	}

	// Remove this cell from array
	bIsDangerous = !Free.Remove(F0);  // if it can't be removed - the bot is standing in the explosion

	// Is there an item nearby?
	if (bIsDangerous == false)
	{
		TSet<FCell> ItemsFromF0 = LevelMap->GetSidesCells(F0, EPathTypesEnum::Safe, 2);
		ItemsFromF0 = LevelMap->IntersectionCellsByTypes(ItemsFromF0, TO_FLAG(EActorTypeEnum::Item));
		if (ItemsFromF0.Num() > 0)
		{
			MoveToCell(ItemsFromF0.Array()[0]);
			return true;
		}
	}
	// ----- Part 1: Cells iterations -----

	// ...

	return true;
}

void AMyAIController::MoveToCell(const FCell& DestinationCell)
{
	if (IS_VALID(MyCharacter) == false)  // The controlled character is not valid or is transient
	{
		return;
	}

	AiMoveTo = DestinationCell;
	MoveToLocation(AiMoveTo.Location, -1.0f, false, false);

	// Rotate the character
	FRotator NewRotation = FRotator::ZeroRotator;
	NewRotation.Yaw = UKismetMathLibrary::FindLookAtRotation(MyCharacter->GetActorLocation(), AiMoveTo.Location).Yaw;
	MyCharacter->SetActorRotation(NewRotation);

#if WITH_EDITOR  // [Editor]
	// Visualize and show destination cell
	if (!USingletonLibrary::IsEditorNotPieWorld())  // [PIE]
	{
		USingletonLibrary::PrintToLog(this, "MoveAI", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(MyCharacter);
	}  // [PIE]

	if (MyCharacter->MapComponent  // is accessible map component
		&& MyCharacter->MapComponent->bShouldShowRenders)
	{
		USingletonLibrary::AddDebugTextRenders(MyCharacter, TSet<FCell>{AiMoveTo}, FLinearColor::Gray, 255, 300, "x");
	}
#endif
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
