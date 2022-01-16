// Copyright 2021 Yevhenii Selivanov.

#include "Controllers/MyAIController.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Globals/SingletonLibrary.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"

// Returns the AI data asset
const UAIDataAsset& UAIDataAsset::Get()
{
	const UAIDataAsset* AIDataAsset = USingletonLibrary::GetAIDataAsset();
	checkf(AIDataAsset, TEXT("The AI Data Asset is not valid"));
	return *AIDataAsset;
}

// Sets default values for this character's properties
AMyAIController::AMyAIController()
{
	// Set this AI controller to don't call Tick()
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bAttachToPawn = true;
}

// Makes AI go toward specified destination cell
void AMyAIController::MoveToCell(const FCell& DestinationCell)
{
	if (!IS_VALID(OwnerInternal)) // The controlled character is not valid or is transient
	{
		return;
	}

	AIMoveToInternal = DestinationCell;
	MoveToLocation(AIMoveToInternal.Location, INDEX_NONE, false, false);

	// Rotate the character
	OwnerInternal->RotateToLocation(AIMoveToInternal.Location, false);

#if WITH_EDITOR	 // [IsEditor]
	if (USingletonLibrary::IsEditor())
	{
		// Visualize and show destination cell
		if (!USingletonLibrary::IsEditorNotPieWorld()) // [PIE]
		{
			USingletonLibrary::ClearOwnerTextRenders(OwnerInternal);
		} // [IsEditorNotPieWorld]

		const UMapComponent* MapComponent = UMapComponent::GetMapComponent(OwnerInternal);
		if (MapComponent // is valid  map component
		    && MapComponent->bShouldShowRenders)
		{
			USingletonLibrary::AddDebugTextRenders(OwnerInternal, {AIMoveToInternal}, FLinearColor::Gray, 255.f, 300.f, TEXT("x"));
		}
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

	Possess(OwnerInternal);
}

//  Called when the game starts or when spawned
void AMyAIController::BeginPlay()
{
	// Call to super
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	// Setup timer handle to update AI brain (initialized being paused)
	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.SetTimer(AIUpdateHandleInternal, this, &ThisClass::UpdateAI, UGeneratedMapDataAsset::Get().GetTickInterval(), true, KINDA_SMALL_NUMBER);
	TimerManager.PauseTimer(AIUpdateHandleInternal);
}

// Allows the PlayerController to set up custom input bindings
void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IS_VALID(InPawn) == false)
	{
		return;
	}

	OwnerInternal = Cast<APlayerCharacter>(InPawn);

	if (!USingletonLibrary::GOnAIUpdatedDelegate.IsBoundToObject(this))
	{
		USingletonLibrary::GOnAIUpdatedDelegate.AddUObject(this, &ThisClass::UpdateAI);
	}

	const bool bMatchStarted = AMyGameStateBase::GetCurrentGameState(this) == ECGS::InGame;
	SetAI(bMatchStarted);
}

// The main AI logic
void AMyAIController::UpdateAI()
{
	const UMapComponent* MapComponent = UMapComponent::GetMapComponent(OwnerInternal);
	if (!IS_VALID(OwnerInternal)   // The controller character is not valid or is transient
	    || !IsValid(MapComponent)) // The Map Component is not valid
	{
		return;
	}

	const AGeneratedMap& LevelMap = AGeneratedMap::Get();
	const UAIDataAsset& AIDataAsset = UAIDataAsset::Get();

#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		USingletonLibrary::ClearOwnerTextRenders(OwnerInternal);
		AIMoveToInternal = FCell::ZeroCell;
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	// ----- Part 0: Before iterations -----

	// Set the START cell searching bot location
	const FCell F0 = MapComponent->Cell;

	// Searching 'SAFE NEIGHBORS'
	static constexpr float MaxInteger = TNumericLimits<int32>::Max();
	FCells Free;
	uint8 bIsDangerous;
	for (bIsDangerous = 0; bIsDangerous <= 1; ++bIsDangerous) // two searches (safe and free)
	{
		LevelMap.GetSidesCells(Free, F0, bIsDangerous ? EPathType::Free : EPathType::Safe, MaxInteger);
		if (!bIsDangerous && Free.Num() > 0)
		{
			// Remove this cell from array
			bIsDangerous = !Free.Remove(F0); // if it can't be removed - the bot is standing in the explosion
			break;
		}
	}

	// Is there an item nearby?
	if (bIsDangerous == false)
	{
		FCells ItemsFromF0;
		LevelMap.GetSidesCells(ItemsFromF0, F0, EPathType::Safe, AIDataAsset.GetItemSearchRadius());
		LevelMap.IntersectCellsByTypes(ItemsFromF0, TO_FLAG(EAT::Item), false);
		if (ItemsFromF0.Num() > 0)
		{
			MoveToCell(ItemsFromF0.Array()[0]);
			return;
		}
	}
	// ----- Part 1: Cells iteration -----

	FCells AllCrossways;    //  cells of all crossways
	FCells SecureCrossways; // crossways without players
	FCells FoundItems;
	bool bIsItemInDirect = false;

	for (auto F = Free.CreateIterator(); F; ++F)
	{
		if (bIsDangerous // is not dangerous situation
		    && USingletonLibrary::CalculateCellsLength(F0, *F) > AIDataAsset.GetNearDangerousRadius())
		{
			F.RemoveCurrent(); // removing distant cells
			continue;
		}

		FCells ThisCrossway;
		LevelMap.GetSidesCells(ThisCrossway, *F, EPathType::Safe, AIDataAsset.GetCrosswaySearchRadius());
		FCells Way = Free;                  // Way = Safe / (Free + F0)
		Way.Emplace(F0);                    // Way = Free + F0
		Way = ThisCrossway.Difference(Way); // Way = Safe / Way

		if (Way.Num() > 0) // Are there any cells?
		{
			// Finding crossways
			AllCrossways.Emplace(*F); // is the crossway
			LevelMap.IntersectCellsByTypes(Way = ThisCrossway, TO_FLAG(EAT::Player), false, MapComponent);
			if (Way.Num() == 0)
			{
				SecureCrossways.Emplace(*F);
			}

			// Finding items
			LevelMap.IntersectCellsByTypes(ThisCrossway, TO_FLAG(EAT::Item), false);
			if (ThisCrossway.Num() > 0) // Is there items in this crossway?
			{
				ThisCrossway = ThisCrossway.Intersect(Free); // ThisCrossway = ThisCrossway ∪ Free
				if (ThisCrossway.Num() > 0)                  // Is there direct items in this crossway?
				{
					if (bIsItemInDirect == false) // is the first found direct item
					{
						bIsItemInDirect = true;
						FoundItems.Empty(); // clear all previously found corner items
					}
					FoundItems = FoundItems.Union(ThisCrossway); // Add found direct items
				}                                                // item around the corner
				else if (bIsItemInDirect == false)               // Need corner item?
				{
					FoundItems.Emplace(*F); // Add found corner item
				}
			} // [has items]
		}     // [is crossway]
		else if (bIsDangerous && ThisCrossway.Contains(*F) == false)
		{
			F.RemoveCurrent(); // In the dangerous situation delete a non-crossway cell
		}
	}

	Free.Compact();
	Free.Shrink();
	if (Free.Num() == 0)
	{
		return;
	}

	// ----- Part 2: Cells filtration -----

	FCells Filtered = FoundItems.Num() > 0 ? FoundItems : Free; // selected cells
	bool bIsFilteringFailed = false;
	static constexpr int32 FilteringStepsNum = 4;
	for (int32 i = 0; i < FilteringStepsNum; ++i)
	{
		FCells FilteringStep;
		switch (i)
		{
			case 0: // All crossways: Filtered ∪ AllCrossways
				FilteringStep = Filtered.Intersect(AllCrossways);
				break;
			case 1: // Without players
				LevelMap.GetSidesCells(FilteringStep, F0, EPathType::Secure, MaxInteger);
				FilteringStep = Filtered.Intersect(FilteringStep);
				break;
			case 2: // Without crossways with another players
				FilteringStep = Filtered.Intersect(SecureCrossways);
				break;
			case 3: // Only nearest cells (length <= near radius)
				for (const FCell& It : Filtered)
				{
					if (USingletonLibrary::CalculateCellsLength(F0, It) <= AIDataAsset.GetNearFilterRadius())
					{
						FilteringStep.Emplace(It);
					}
				}
				break;
			default:
				break;
		}

		if (FilteringStep.Num() > 0)
		{
			Filtered = FilteringStep;
		}
		else
		{
			bIsFilteringFailed = true;
		}
	} // [Loopy filtering]

	// ----- Part 2: Deciding whether to put the bomb -----

	if (bIsDangerous == false          // is not dangerous situation
	    && bIsFilteringFailed == false // filtering was not failed
	    && bIsItemInDirect == false)   // was not found direct items
	{
		FCells BoxesAndPlayers;
		LevelMap.GetSidesCells(BoxesAndPlayers, F0, EPathType::Explosion, OwnerInternal->GetPowerups().FireN);
		LevelMap.IntersectCellsByTypes(BoxesAndPlayers, TO_FLAG(EAT::Box | EAT::Player), false, MapComponent);
		if (BoxesAndPlayers.Num() > 0) // Are bombs or players in own bomb radius
		{
			OwnerInternal->SpawnBomb();
			Free.Empty(); // Delete all cells to make new choice

#if WITH_EDITOR	 // [Editor]
			if (MapComponent->bShouldShowRenders)
			{
				USingletonLibrary::AddDebugTextRenders(OwnerInternal, {F0}, FLinearColor::Red, 261.F, 95.F, TEXT("Attack"));
			}
#endif	// [Editor]
		}
	}

	// ----- Part 3: Making choice-----

	if (Free.Contains(AIMoveToInternal))
	{
		return;
	}

	MoveToCell(Filtered.Array()[FMath::RandRange(NULL, Filtered.Num() - 1)]);

#if WITH_EDITOR	 // [Editor]
	if (MapComponent->bShouldShowRenders)
	{
		static constexpr int32 VisualizationTypesNum = 3;
		for (int32 Index = 0; Index < VisualizationTypesNum; ++Index)
		{
			FCells VisualizingStep;
			FLinearColor Color;
			FString String = "+";
			FVector Position = FVector::ZeroVector;
			switch (Index)
			{
				case 0:
				{
					VisualizingStep = AllCrossways.Difference(SecureCrossways);
					Color = FLinearColor::Red;
					break;
				}
				case 1:
				{
					VisualizingStep = SecureCrossways;
					Color = FLinearColor::Green;
					break;
				}
				case 2:
				{
					VisualizingStep = Filtered;
					Color = FLinearColor::Yellow;
					String = TEXT("F");
					static const FVector DefaultPosition(-50.0F, -50.0F, 0.0F);
					Position = DefaultPosition;
					break;
				}
				default:
					break;
			}
			USingletonLibrary::AddDebugTextRenders(OwnerInternal, VisualizingStep, Color, 263.f, 124.f, String, Position);
		} // [Loopy visualization]
	}
#endif	// [Editor]
}

// Enable or disable AI for this bot
void AMyAIController::SetAI(bool bShouldEnable) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	if (bShouldEnable)
	{
		TimerManager.UnPauseTimer(AIUpdateHandleInternal);
	}
	else
	{
		TimerManager.PauseTimer(AIUpdateHandleInternal);
	}
}

// Listen game states to enable or disable AI
void AMyAIController::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		case ECurrentGameState::GameStarting:
		case ECurrentGameState::EndGame:
		{
			SetAI(false);
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetAI(true);
			break;
		}
		default:
			break;
	}
}
