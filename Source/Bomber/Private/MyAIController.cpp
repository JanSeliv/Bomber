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
	bool& bIsDangerous,
	TSet<FCell>& AllCrossways,
	TSet<FCell>& SecureCrossways,
	TSet<FCell>& FoundItems,
	bool& bIsItemInDirect,
	TSet<FCell>& Filtered,
	bool& bIsFilteringFailed)
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
	F0 = FCell(MyCharacter);

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
	// ----- Part 1: Cells iteration -----

	for (const FCell& F : Free)
	{
		if (bIsDangerous  // is not dangerous situation
			&& USingletonLibrary::CalculateCellsLength(F0, F) > 3.0F)
		{
			Free.Remove(F);  // removing distant cells
			continue;
		}

		// Way = Safe / (Free + F0)
		TSet<FCell> ThisCrossway = LevelMap->GetSidesCells(F, EPathTypesEnum::Safe, 2);
		TSet<FCell> Way = Free;
		Way.Add(F0);						 // Way = Free + F0
		Way = ThisCrossway.Difference(Way);  // Way = Safe / Way

		if (Way.Num() > 0)  // Are there any cells?
		{
			// Finding crossways
			AllCrossways.Add(F);  // is the crossway
			if (LevelMap->IntersectionCellsByTypes(ThisCrossway, TO_FLAG(EActorTypeEnum::Player), MyCharacter).Num() == 0)
			{
				SecureCrossways.Add(F);
			}

			// Finding items
			ThisCrossway = LevelMap->IntersectionCellsByTypes(ThisCrossway, TO_FLAG(EActorTypeEnum::Item));
			if (ThisCrossway.Num() > 0)  // Is there items in this crossway?
			{
				ThisCrossway = ThisCrossway.Intersect(Free);  // ThisCrossway = ThisCrossway ∪ Free
				if (ThisCrossway.Num() > 0)					  // Is there direct items in this crossway?
				{
					if (bIsItemInDirect == false)  // is the first found direct item
					{
						bIsItemInDirect = true;
						FoundItems.Empty();  // clear all previously found corner items
					}
					FoundItems = FoundItems.Union(ThisCrossway);  // Add found direct items
				}												  // item around the corner
				else if (bIsItemInDirect == false)				  // Need corner item?
				{
					FoundItems.Add(F);  // Add found corner item
				}
			}  // [has items]
		}	  // [is crossway]
		else if (bIsDangerous && ThisCrossway.Contains(F) == false)
		{
			Free.Remove(F);  // In the dangerous situation delete a non-crossway cell
		}
	}

	if (Free.Num() == 0)
	{
		return false;
	}

	// ----- Part 2: Cells filtration -----

	Filtered = (FoundItems.Num() > 0 ? FoundItems : Free);  //TSet<FCell> Filtered;
	for (int32 i = 0; i < 4; ++i)
	{
		TSet<FCell> FilteringStep;
		switch (i)  // 4 filtering steps
		{
			case 0:  // All crossways: Filtered ∪ AllCrossways
				FilteringStep = Filtered.Intersect(AllCrossways);
				break;
			case 1:  // Without players
				FilteringStep = Filtered.Intersect(LevelMap->GetSidesCells(F0, EPathTypesEnum::Secure));
				break;
			case 2:  // Without crossways with another players
				FilteringStep = Filtered.Intersect(SecureCrossways);
				break;
			case 3:  // Only nearest cells ( length <= 3 cells)
				for (const FCell& It : Filtered)
				{
					if (USingletonLibrary::CalculateCellsLength(F0, It) <= 3.0F)
					{
						FilteringStep.Add(It);
					}
				}
				break;
			default:
				break;
		}  //[4 filtering steps]

		if (FilteringStep.Num() > 0)
		{
			Filtered = FilteringStep;
		}
		else
		{
			bIsFilteringFailed = true;
		}
	}  // [Filtering]

	// ----- Part 2: Deciding whether to put the bomb -----

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
