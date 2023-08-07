// Copyright (c) Yevhenii Selivanov.

#include "Controllers/MyAIController.h"
//---
#include "Bomber.h"
#include "Components/MapComponent.h"
#include "DataAssets/AIDataAsset.h"
#include "DataAssets/GameStateDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "TimerManager.h"
#include "Components/GameFrameworkComponentManager.h"
//---
#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyAIController)

// Enable or disable all bots
static TAutoConsoleVariable<bool> CVarAISetEnabled(
	TEXT("Bomber.AI.SetEnabled"),
	true,
	TEXT("Enable or disable all bots: 1 (Enable) OR 0 (Disable)"),
	ECVF_Default);

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
	if (!OwnerInternal)
	{
		return;
	}

	if (!IsMoveInputIgnored())
	{
		AIMoveToInternal = DestinationCell;
		MoveToLocation(AIMoveToInternal.Location, INDEX_NONE, false, false);
	}

#if WITH_EDITOR	 // [IsEditor]
	if (FEditorUtilsLibrary::IsEditor())
	{
		// Visualize and show destination cell
		if (UMyBlueprintFunctionLibrary::HasWorldBegunPlay()) // PIE
		{
			UCellsUtilsLibrary::ClearDisplayedCells(OwnerInternal);
		}

		const UMapComponent* MapComponent = UMapComponent::GetMapComponent(OwnerInternal);
		if (MapComponent // is valid  map component
		    && MapComponent->bShouldShowRenders)
		{
			static const FDisplayCellsParams DisplayParams{FLinearColor::Gray, 255.f, 300.f, TEXT("x")};
			UCellsUtilsLibrary::DisplayCell(OwnerInternal, DestinationCell, DisplayParams);
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

// This is called only in the gameplay before calling begin play
void AMyAIController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register controller to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

//  Called when the game starts or when spawned
void AMyAIController::BeginPlay()
{
	// Call to super
	Super::BeginPlay();

	// Setup timer handle to update AI brain (initialized being paused)
	constexpr bool bInLoop = true;
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(AIUpdateHandleInternal, this, &ThisClass::UpdateAI, UGameStateDataAsset::Get().GetTickInterval(), bInLoop, KINDA_SMALL_NUMBER);
	TimerManager.PauseTimer(AIUpdateHandleInternal);
}

// Allows the controller to react on possessing the pawn
void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	OwnerInternal = Cast<APlayerCharacter>(InPawn);

#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()
	    && !UMyUnrealEdEngine::GOnAIUpdatedDelegate.IsBoundToObject(this))
	{
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.AddUObject(this, &ThisClass::UpdateAI);
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	// Listen states
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);

		// Handle current game state if initialized with delay
		if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
		{
			OnGameStateChanged(ECurrentGameState::Menu);
		}
	}

	const bool bMatchStarted = AMyGameStateBase::GetCurrentGameState() == ECGS::InGame;
	SetAI(bMatchStarted);
}

// Allows the controller to react on unpossessing the pawn
void AMyAIController::OnUnPossess()
{
	Super::OnUnPossess();

	OwnerInternal = nullptr;

#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.RemoveAll(this);
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	SetAI(false);
}

// Locks or unlocks movement input
void AMyAIController::SetIgnoreMoveInput(bool bShouldIgnore)
{
	// Do not call super to avoid stacking, override it

	IgnoreMoveInput = bShouldIgnore;
}

// Stops running to target
void AMyAIController::Reset()
{
	// Abort current movement task
	Super::Reset();

	// Reset target location
	AIMoveToInternal = FCell::InvalidCell;
}

// The main AI logic
void AMyAIController::UpdateAI()
{
	const UMapComponent* MapComponent = UMapComponent::GetMapComponent(OwnerInternal);
	if (!OwnerInternal
	    || !IsValid(MapComponent)
	    || !CVarAISetEnabled.GetValueOnAnyThread()) // AI is disabled
	{
		return;
	}

	const UAIDataAsset& AIDataAsset = UAIDataAsset::Get();

#if WITH_EDITOR
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UCellsUtilsLibrary::ClearDisplayedCells(OwnerInternal);
		AIMoveToInternal = FCell::InvalidCell;
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	// ----- Part 0: Before iterations -----

	// Set the START cell searching bot location
	const FCell& F0 = MapComponent->GetCell();

	// Searching 'SAFE NEIGHBORS'
	static constexpr int32 MaxInteger = TNumericLimits<int32>::Max();
	FCells Free;
	uint8 bIsDangerous;
	for (bIsDangerous = 0; bIsDangerous <= 1; ++bIsDangerous) // two searches (safe and free)
	{
		Free = UCellsUtilsLibrary::GetCellsAround(F0, bIsDangerous ? EPathType::Free : EPathType::Safe, MaxInteger);
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
		const FCells ItemsFromF0 = UCellsUtilsLibrary::GetCellsAroundWithActors(F0, EPathType::Safe, AIDataAsset.GetItemSearchRadius(), TO_FLAG(EAT::Item));
		if (!ItemsFromF0.IsEmpty())
		{
			MoveToCell(FCell::GetFirstCellInSet(ItemsFromF0));
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
		    && FCell::Distance<float>(F0, *F) > AIDataAsset.GetNearDangerousRadius())
		{
			F.RemoveCurrent(); // removing distant cells
			continue;
		}

		const FCells ThisCrossway = UCellsUtilsLibrary::GetCellsAround(*F, EPathType::Safe, AIDataAsset.GetCrosswaySearchRadius());
		FCells Way = Free;                  // Way = Safe / (Free + F0)
		Way.Emplace(F0);                    // Way = Free + F0
		Way = ThisCrossway.Difference(Way); // Way = Safe / Way

		if (Way.Num() > 0) // Are there any cells?
		{
			// Finding crossways
			AllCrossways.Emplace(*F); // is the crossway
			Way = UCellsUtilsLibrary::FilterCellsByActors(ThisCrossway, TO_FLAG(EAT::Player));
			Way.Remove(MapComponent->GetCell());
			if (Way.Num() == 0)
			{
				SecureCrossways.Emplace(*F);
			}

			// Finding items
			FCells ItemsAround = UCellsUtilsLibrary::FilterCellsByActors(ThisCrossway, TO_FLAG(EAT::Item));
			if (ItemsAround.Num() > 0) // Is there items in this crossway?
			{
				ItemsAround = ItemsAround.Intersect(Free); // ItemsAround = ItemsAround ∪ Free
				if (ItemsAround.Num() > 0)                 // Is there direct items in this crossway?
				{
					if (bIsItemInDirect == false) // is the first found direct item
					{
						bIsItemInDirect = true;
						FoundItems.Empty(); // clear all previously found corner items
					}
					FoundItems = FoundItems.Union(ItemsAround); // Add found direct items
				}                                               // item around the corner
				else if (bIsItemInDirect == false)              // Need corner item?
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
	for (int32 Index = 0; Index < FilteringStepsNum; ++Index)
	{
		FCells FilteringStep;
		switch (Index)
		{
			case 0: // All crossways: Filtered ∪ AllCrossways
				FilteringStep = Filtered.Intersect(AllCrossways);
				break;
			case 1: // Without players
				FilteringStep = UCellsUtilsLibrary::GetCellsAround(F0, EPathType::Secure, MaxInteger);
				FilteringStep = Filtered.Intersect(FilteringStep);
				break;
			case 2: // Without crossways with another players
				FilteringStep = Filtered.Intersect(SecureCrossways);
				break;
			case 3: // Only nearest cells (length <= near radius)
				for (const FCell& It : Filtered)
				{
					if (FCell::Distance<float>(F0, It) <= AIDataAsset.GetNearFilterRadius())
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
		FCells BoxesAndPlayers = UCellsUtilsLibrary::GetCellsAroundWithActors(F0, EPathType::Explosion, OwnerInternal->GetPowerups().FireN, TO_FLAG(EAT::Box | EAT::Player));
		BoxesAndPlayers.Remove(MapComponent->GetCell());
		if (BoxesAndPlayers.Num() > 0) // Are bombs or players in own bomb radius
		{
			OwnerInternal->ServerSpawnBomb();
			Free.Empty(); // Delete all cells to make new choice

#if WITH_EDITOR	 // [Editor]
			if (MapComponent->bShouldShowRenders)
			{
				static const FDisplayCellsParams DisplayParams{FLinearColor::Red, 261.F, 95.F, TEXT("Attack")};
				UCellsUtilsLibrary::DisplayCell(OwnerInternal, F0, DisplayParams);
			}
#endif	// [Editor]
		}
	}

	// ----- Part 3: Making choice-----

	if (Free.Contains(AIMoveToInternal))
	{
		return;
	}

	MoveToCell(Filtered.Array()[FMath::RandRange(0, Filtered.Num() - 1)]);

#if WITH_EDITOR	 // [Editor]
	if (MapComponent->bShouldShowRenders)
	{
		static constexpr int32 VisualizationTypesNum = 3;
		for (int32 Index = 0; Index < VisualizationTypesNum; ++Index)
		{
			FCells VisualizingStep;
			FLinearColor Color;
			FName Symbol = TEXT("+");
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
					Symbol = TEXT("F");
					static const FVector DefaultPosition(-50.0F, -50.0F, 0.0F);
					Position = DefaultPosition;
					break;
				}
				default:
					break;
			}
			const FDisplayCellsParams DisplayParams{Color, 263.f, 124.f, Symbol, Position};
			UCellsUtilsLibrary::DisplayCells(OwnerInternal, VisualizingStep, DisplayParams);
		} // [Loopy visualization]
	}
#endif	// [Editor]
}

// Enable or disable AI for this bot
void AMyAIController::SetAI(bool bShouldEnable)
{
	const bool bWantsEnableDeadAI = !OwnerInternal && bShouldEnable;
	if (bWantsEnableDeadAI
	    || !HasAuthority())
	{
		return;
	}

	Reset();

	SetIgnoreMoveInput(!bShouldEnable);

	// Handle the Ai updating timer
	FTimerManager& TimerManager = GetWorldTimerManager();
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
