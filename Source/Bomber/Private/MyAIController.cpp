// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAIController.h"

#include "Kismet/KismetMathLibrary.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"

// Sets default values for this character's properties
AMyAIController::AMyAIController()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 0.25F;

	bAttachToPawn = true;
}

// The main AI logic
bool AMyAIController::UpdateAI_Implementation()
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
	const FCell F0(MyCharacter);

	// Searching 'SAFE NEIGHBORS'
	TSet<FCell> Free;
	bool bIsDangerous;
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

	TSet<FCell> AllCrossways;	 //  cells of all crossways
	TSet<FCell> SecureCrossways;  // crossways without players
	TSet<FCell> FoundItems;
	bool bIsItemInDirect = false;

	for (auto F = Free.CreateIterator(); F; ++F)
	{
		if (bIsDangerous  // is not dangerous situation
			&& USingletonLibrary::CalculateCellsLength(F0, *F) > 3.0F)
		{
			Free.Remove(*F);  // removing distant cells
			continue;
		}

		TSet<FCell> ThisCrossway = LevelMap->GetSidesCells(*F, EPathTypesEnum::Safe, 2);
		TSet<FCell> Way = Free;				 // Way = Safe / (Free + F0)
		Way.Add(F0);						 // Way = Free + F0
		Way = ThisCrossway.Difference(Way);  // Way = Safe / Way

		if (Way.Num() > 0)  // Are there any cells?
		{
			// Finding crossways
			AllCrossways.Add(*F);  // is the crossway
			if (LevelMap->IntersectionCellsByTypes(ThisCrossway, TO_FLAG(EActorTypeEnum::Player), MyCharacter).Num() == 0)
			{
				SecureCrossways.Add(*F);
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
					FoundItems.Add(*F);  // Add found corner item
				}
			}  // [has items]
		}	  // [is crossway]
		else if (bIsDangerous && ThisCrossway.Contains(*F) == false)
		{
			Free.Remove(*F);  // In the dangerous situation delete a non-crossway cell
		}
	}

	Free.Compact();
	Free.Shrink();
	if (Free.Num() == 0)
	{
		return false;
	}

	// ----- Part 2: Cells filtration -----

	TSet<FCell> Filtered = FoundItems.Num() > 0 ? FoundItems : Free;  // selected cells
	bool bIsFilteringFailed = false;
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
		TSet<FCell> BoxesAndPlayers = LevelMap->GetSidesCells(F0, EPathTypesEnum::Explosion, MyCharacter->Powerups_.FireN);
		BoxesAndPlayers = LevelMap->IntersectionCellsByTypes(BoxesAndPlayers, TO_FLAG(EActorTypeEnum::Player | EActorTypeEnum::Box), MyCharacter);
		if (BoxesAndPlayers.Num() > 0)  // Are bombs or players in own bomb radius
		{
			MyCharacter->SpawnBomb();
			Free.Empty();  // Delete all cells to make new choice

#if WITH_EDITOR  // [Editor]
			if (MyCharacter->MapComponent->bShouldShowRenders)
			{
				USingletonLibrary::AddDebugTextRenders(MyCharacter, TSet<FCell>{F0}, FLinearColor::Red, 263, 95, "Attack");
			}
#endif  // [Editor]
		}
	}

	// ----- Part 3: Making choice-----

	if (Free.Contains(AiMoveTo))
	{
		return false;
	}

	MoveToCell(Filtered.Array()[FMath::RandRange(NULL, Filtered.Num() - 1)]);

#if WITH_EDITOR  // [Editor]
	for (int32 i = 0; i < 3; ++i)
	{
		TSet<FCell> VisualizingStep;
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
				VisualizingStep = Free;
				Color = FLinearColor::Green;
				break;
			case 2:
				VisualizingStep = Filtered;
				Color = FLinearColor::Yellow;
				String = "F";
				Position = FVector(-50.0F, -50.0F, 0.0F);
				break;
			default: break;
		}  // [3 visualization types]
		if (MyCharacter->MapComponent->bShouldShowRenders)
		{
			USingletonLibrary::AddDebugTextRenders(MyCharacter, VisualizingStep, Color, 263, 124, String, Position);
		}
	}   // [Loopy visualization]
#endif  // [Editor]

	return true;
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
		USingletonLibrary::AddDebugTextRenders(MyCharacter, TSet<FCell>{AiMoveTo}, FLinearColor::Gray, 255, 300, "x");
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

	AiMoveTo = FCell::ZeroCell;
}

// Function called every frame on this AI controller to update movement
void AMyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAI();
}

// Allows the PlayerController to set up custom input bindings
void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IS_VALID(InPawn) == false)
	{
		return;
	}

	MyCharacter = Cast<AMyCharacter>(InPawn);
}
