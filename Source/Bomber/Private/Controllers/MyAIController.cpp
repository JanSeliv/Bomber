// Copyright 2019 Yevhenii Selivanov.

#include "MyAIController.h"

#include "Kismet/KismetMathLibrary.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "LevelActors/PlayerCharacter.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"

// Sets default values for this character's properties
AMyAIController::AMyAIController()
{
	// Set this AI controller to don't call Tick()
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bAttachToPawn = true;
}

// The main AI logic
void AMyAIController::UpdateAI()
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap								 // The Level Map is not valid or is transient
		|| !IS_VALID(MyCharacter)				 // The controller character is not valid or is transient
		|| !IsValid(MyCharacter->MapComponent))  // The Map Component is not valid
	{
		return;
	}

#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld())  // [IsEditorNotPieWorld]
	{
		USingletonLibrary::PrintToLog(this, "[IsEditorNotPieWorld]UpdateAI", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(MyCharacter);
		AiMoveTo.SetToZero();
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]

	// ----- Part 0: Before iterations -----

	// Set the START cell searching bot location
	const FCell F0 = MyCharacter->MapComponent->GetCell();

	// Searching 'SAFE NEIGHBORS'
	FCells Free;
	int32 bIsDangerous;
	for (bIsDangerous = 0; bIsDangerous <= 1; ++bIsDangerous)  // two searches (safe and free)
	{
		LevelMap->GetSidesCells(Free, F0, bIsDangerous ? EPathType::Free : EPathType::Safe, MAX_int32);
		if (Free.Num() > 0)
		{
			break;
		}
	}

	// Remove this cell from array
	bIsDangerous = !Free.Remove(F0);  // if it can't be removed - the bot is standing in the explosion

	// Is there an item nearby?
	if (bIsDangerous == false)
	{
		FCells ItemsFromF0;
		LevelMap->GetSidesCells(ItemsFromF0, F0, EPathType::Safe, 2);
		LevelMap->IntersectCellsByTypes(ItemsFromF0, TO_FLAG(EActorType::Item), false);
		if (ItemsFromF0.Num() > 0)
		{
			MoveToCell(ItemsFromF0.Array()[0]);
			return;
		}
	}
	// ----- Part 1: Cells iteration -----

	FCells AllCrossways;	 //  cells of all crossways
	FCells SecureCrossways;  // crossways without players
	FCells FoundItems;
	bool bIsItemInDirect = false;

	for (auto F = Free.CreateIterator(); F; ++F)
	{
		if (bIsDangerous  // is not dangerous situation
			&& USingletonLibrary::CalculateCellsLength(F0, *F) > 3.0F)
		{
			F.RemoveCurrent();  // removing distant cells
			continue;
		}

		FCells ThisCrossway;
		LevelMap->GetSidesCells(ThisCrossway, *F, EPathType::Safe, 2);
		FCells Way = Free;					 // Way = Safe / (Free + F0)
		Way.Emplace(F0);					 // Way = Free + F0
		Way = ThisCrossway.Difference(Way);  // Way = Safe / Way

		if (Way.Num() > 0)  // Are there any cells?
		{
			// Finding crossways
			AllCrossways.Emplace(*F);  // is the crossway
			LevelMap->IntersectCellsByTypes(Way = ThisCrossway, TO_FLAG(EActorType::Player), false, MyCharacter->MapComponent);
			if (Way.Num() == 0)
			{
				SecureCrossways.Emplace(*F);
			}

			// Finding items
			LevelMap->IntersectCellsByTypes(ThisCrossway, TO_FLAG(EActorType::Item), false);
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
					FoundItems.Emplace(*F);  // Add found corner item
				}
			}  // [has items]
		}	  // [is crossway]
		else if (bIsDangerous && ThisCrossway.Contains(*F) == false)
		{
			F.RemoveCurrent();  // In the dangerous situation delete a non-crossway cell
		}
	}

	Free.Compact();
	Free.Shrink();
	if (Free.Num() == 0)
	{
		return;
	}

	// ----- Part 2: Cells filtration -----

	FCells Filtered = FoundItems.Num() > 0 ? FoundItems : Free;  // selected cells
	bool bIsFilteringFailed = false;
	for (int32 i = 0; i < 4; ++i)
	{
		FCells FilteringStep;
		switch (i)  // 4 filtering steps
		{
			case 0:  // All crossways: Filtered ∪ AllCrossways
				FilteringStep = Filtered.Intersect(AllCrossways);
				break;
			case 1:  // Without players
				LevelMap->GetSidesCells(FilteringStep, F0, EPathType::Secure, MAX_int32);
				FilteringStep = Filtered.Intersect(FilteringStep);
				break;
			case 2:  // Without crossways with another players
				FilteringStep = Filtered.Intersect(SecureCrossways);
				break;
			case 3:  // Only nearest cells ( length <= 3 cells)
				for (const FCell& It : Filtered)
				{
					if (USingletonLibrary::CalculateCellsLength(F0, It) <= 3.0F)
					{
						FilteringStep.Emplace(It);
					}
				}
				break;
			default: break;
		}  //[4 filtering steps]

		if (FilteringStep.Num() > 0)
		{
			Filtered = FilteringStep;
		}
		else
		{
			bIsFilteringFailed = true;
		}
	}  // [Loopy filtering]

	// ----- Part 2: Deciding whether to put the bomb -----

	if (bIsDangerous == false			// is not dangerous situation
		&& bIsFilteringFailed == false  // filtering was not failed
		&& bIsItemInDirect == false)	// was not found direct items
	{
		FCells BoxesAndPlayers;
		LevelMap->GetSidesCells(BoxesAndPlayers, F0, EPathType::Explosion, MyCharacter->Powerups_.FireN);
		LevelMap->IntersectCellsByTypes(BoxesAndPlayers, TO_FLAG(EActorType::Box | EActorType::Player), false, MyCharacter->MapComponent);
		if (BoxesAndPlayers.Num() > 0)  // Are bombs or players in own bomb radius
		{
			MyCharacter->SpawnBomb();
			Free.Empty();  // Delete all cells to make new choice

#if WITH_EDITOR  // [Editor]
			if (MyCharacter->MapComponent->bShouldShowRenders)
			{
				USingletonLibrary::AddDebugTextRenders(MyCharacter, FCells{F0}, FLinearColor::Red, 263, 95, "Attack");
			}
#endif  // [Editor]
		}
	}

	// ----- Part 3: Making choice-----

	if (Free.Contains(AiMoveTo))
	{
		return;
	}

	MoveToCell(Filtered.Array()[FMath::RandRange(NULL, Filtered.Num() - 1)]);

#if WITH_EDITOR  // [Editor]
	if (MyCharacter->MapComponent->bShouldShowRenders)
	{
		for (int32 i = 0; i < 3; ++i)
		{
			FCells VisualizingStep;
			FLinearColor Color;
			FString String = "+";
			FVector Position = FVector::ZeroVector;
			switch (i)  // 3 types of visualization
			{
				case 0:
					VisualizingStep = AllCrossways.Difference(SecureCrossways);
					Color = FLinearColor::Red;
					break;
				case 1:
					VisualizingStep = SecureCrossways;
					Color = FLinearColor::Green;
					break;
				case 2:
					VisualizingStep = Filtered;
					Color = FLinearColor::Yellow;
					String = "F";
					Position = FVector(-50.0F, -50.0F, 0.0F);
					break;
				default: break;
			}  // 3 visualization types

			USingletonLibrary::AddDebugTextRenders(MyCharacter, VisualizingStep, Color, 263, 124, String, Position);
		}  // [Loopy visualization]
	}
#endif  // [Editor]
}

// Makes AI go toward specified destination cell
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
		USingletonLibrary::AddDebugTextRenders(MyCharacter, FCells{AiMoveTo}, FLinearColor::Gray, 255, 300, "x");
	}
#endif
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when an instance of this class is placed (in editor) or spawned
void AMyAIController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IS_TRANSIENT(this))
	{
		return;
	}

	Possess(MyCharacter);
}

//  Called when the game starts or when spawned
void AMyAIController::BeginPlay()
{
	Super::BeginPlay();
}

// Allows the PlayerController to set up custom input bindings
void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IS_VALID(InPawn) == false)
	{
		return;
	}

	MyCharacter = Cast<APlayerCharacter>(InPawn);
	USingletonLibrary::GetSingleton()->OnAIUpdatedDelegate.AddUniqueDynamic(this, &ThisClass::UpdateAI);
}
