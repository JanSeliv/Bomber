// Copyright (c) Yevhenii Selivanov.

#include "Controllers/MyAIController.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Bomber.h"
#include "Components/BmrMoverComponent.h"
#include "Components/MapComponent.h"
#include "DataAssets/AIDataAsset.h"
#include "DataAssets/GameStateDataAsset.h"
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"

#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#endif

// UE
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MyAIController)

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
	APlayerCharacter* InOwner = GetPawn<APlayerCharacter>();
	const UMapComponent* MapComponent = UMapComponent::GetMapComponent(InOwner);
	UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	if (!MapComponent
	    || !MoverComponent)
	{
		return;
	}

	if (!MoverComponent->IsBlockedMovement())
	{
		const FCell& CurrentCell = MapComponent->GetCell();
		const bool bHasArrived = CurrentCell == DestinationCell;
		AIMoveToInternal = bHasArrived ? FCell::InvalidCell : DestinationCell;

		// AI is moving directly in desired direction without navmesh usage (instead of MoveToLocation with navmesh)
		const FVector Direction = bHasArrived ? FVector::ZeroVector : (DestinationCell.Location - InOwner->GetActorLocation()).GetSafeNormal2D();
		MoverComponent->RequestMoveByIntent(Direction);
	}

#if WITH_EDITOR // [IsEditor]
	if (UUtilsLibrary::IsEditor())
	{
		// Visualize and show destination cell
		if (UUtilsLibrary::HasWorldBegunPlay()) // PIE
		{
			UCellsUtilsLibrary::ClearDisplayedCells(InOwner);
		}

		if (MapComponent->bShouldShowRenders)
		{
			static const FDisplayCellsParams DisplayParams{FLinearColor::Gray, 255.f, 300.f, TEXT("x")};
			UCellsUtilsLibrary::DisplayCell(InOwner, DestinationCell, DisplayParams);
		}
	}
#endif
}

// Returns true if AI is enabled (move input is not ignored and cheat is not enabled)
bool AMyAIController::IsAIEnabled() const
{
	const APlayerCharacter* InOwner = GetPawn<APlayerCharacter>();
	const UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	return MoverComponent
	       && !MoverComponent->IsBlockedMovement()
	       && UMyCheatManager::CVarAISetEnabled.GetValueOnAnyThread();
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// This is called only in the gameplay before calling begin play
void AMyAIController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register controller to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// Allows the controller to react on possessing the pawn
void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APlayerCharacter* InOwner = Cast<APlayerCharacter>(InPawn);
	if (!InPawn)
	{
		return;
	}

#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		if (!UMyUnrealEdEngine::GOnAIUpdatedDelegate.IsBoundToObject(this))
		{
			UMyUnrealEdEngine::GOnAIUpdatedDelegate.AddUObject(this, &ThisClass::UpdateAI);
		}

		// ! It's editor not Pie World, don't continue further runtime logic
		return;
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	if (GetPlayerState<AMyPlayerState>() == nullptr)
	{
		// Spawn Player State for AI to replicate game-relevant info like scores, teams etc
		InitPlayerState();
		AMyPlayerState* NewPlayerState = GetPlayerState<AMyPlayerState>();
		checkf(NewPlayerState, TEXT("ERROR: [%i] %s:\n'NewPlayerState' was not spawned!"), __LINE__, *FString(__FUNCTION__));
		InOwner->SetPlayerState(NewPlayerState);

		// Update default nickname for AI
		NewPlayerState->SetDefaultPlayerName();
	}

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// Notify host about bot possession
	UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.Broadcast_OnCharacterPossessed(*InOwner);

	UMapComponent* MapComponent = UMapComponent::GetMapComponent(InOwner);
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);

	// Subscribe to movement completion to trigger AI updates
	UBmrMoverComponent* MoverComponent = InOwner->GetMoverComponent();
	checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
	MoverComponent->OnPostSimulationTick.AddUniqueDynamic(this, &ThisClass::OnOwnerMovementCompleted);

	const bool bMatchStarted = AMyGameStateBase::GetCurrentGameState() == ECGS::InGame;
	SetAI(bMatchStarted);
}

// Allows the controller to react on unpossessing the pawn
void AMyAIController::OnUnPossess()
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.RemoveAll(this);
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	SetAI(false);

	if (const APlayerCharacter* InOwner = GetPawn<APlayerCharacter>())
	{
		UMapComponent* MapComponent = UMapComponent::GetMapComponent(InOwner);
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
		MapComponent->OnPostRemovedFromLevel.RemoveAll(this);

		UBmrMoverComponent* MoverComponent = InOwner->GetMoverComponent();
		checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
		MoverComponent->OnPostSimulationTick.AddUniqueDynamic(this, &ThisClass::OnOwnerMovementCompleted);
	}

	Super::OnUnPossess();
}

// Stops running to target
void AMyAIController::Reset()
{
	// Abort current movement task
	Super::Reset();

	// Reset target location
	AIMoveToInternal = FCell::InvalidCell;

	const APlayerCharacter* InOwner = GetPawn<APlayerCharacter>();
	UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	if (MoverComponent)
	{
		MoverComponent->RequestMoveByIntent(FVector::ZeroVector);
	}
}

// The main AI logic
void AMyAIController::UpdateAI()
{
	APlayerCharacter* InOwner = GetPawn<APlayerCharacter>();
	const UMapComponent* MapComponent = InOwner ? UMapComponent::GetMapComponent(InOwner) : nullptr;
	if (!MapComponent
	    || !IsAIEnabled())
	{
		return;
	}

	// Throttle AI updates to match desired tick rate
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeSinceLastUpdate = CurrentTime - LastAIUpdateTimeInternal;
	if (TimeSinceLastUpdate < UGameStateDataAsset::Get().GetTickInterval())
	{
		return;
	}
	LastAIUpdateTimeInternal = CurrentTime;

	// Stop movement if arrived at destination
	if (AIMoveToInternal.IsValid()
	    && MapComponent->GetCell() == AIMoveToInternal)
	{
		Reset();
		// Fall through to choose new destination
	}

	const UAIDataAsset& AIDataAsset = UAIDataAsset::Get();

	if (UUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UCellsUtilsLibrary::ClearDisplayedCells(InOwner);
		AIMoveToInternal = FCell::InvalidCell;
	}

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

	FCells AllCrossways; //  cells of all crossways
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
		FCells Way = Free; // Way = Safe / (Free + F0)
		Way.Emplace(F0); // Way = Free + F0
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
				if (ItemsAround.Num() > 0) // Is there direct items in this crossway?
				{
					if (bIsItemInDirect == false) // is the first found direct item
					{
						bIsItemInDirect = true;
						FoundItems.Empty(); // clear all previously found corner items
					}
					FoundItems = FoundItems.Union(ItemsAround); // Add found direct items
				} // item around the corner
				else if (bIsItemInDirect == false) // Need corner item?
				{
					FoundItems.Emplace(*F); // Add found corner item
				}
			} // [has items]
		} // [is crossway]
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

	if (bCanSpawnBombs // false meaning manually disabled
	    && !bIsDangerous // is not dangerous situation
	    && !bIsFilteringFailed // filtering was not failed
	    && !bIsItemInDirect) // was not found direct items
	{
		const float Fire = UBmrPowerupsAttributeSet::Get(InOwner).GetPowerup_Fire();
		FCells BoxesAndPlayers = UCellsUtilsLibrary::GetCellsAroundWithActors(F0, EPathType::Explosion, Fire, TO_FLAG(EAT::Box | EAT::Player));
		BoxesAndPlayers.Remove(MapComponent->GetCell());
		if (BoxesAndPlayers.Num() > 0) // Are bombs or players in own bomb radius
		{
			InOwner->SpawnBomb();
			Free.Empty(); // Delete all cells to make new choice

#if WITH_EDITOR // [Editor]
			if (MapComponent->bShouldShowRenders)
			{
				static const FDisplayCellsParams DisplayParams{FLinearColor::Red, 261.F, 95.F, TEXT("Attack")};
				UCellsUtilsLibrary::DisplayCell(InOwner, F0, DisplayParams);
			}
#endif // [Editor]
		}
	}

	// ----- Part 3: Making choice-----

	if (Free.Contains(AIMoveToInternal))
	{
		return;
	}

	MoveToCell(Filtered.Array()[FMath::RandRange(0, Filtered.Num() - 1)]);

#if WITH_EDITOR // [Editor]
	if (MapComponent->bShouldShowRenders)
	{
		static constexpr int32 VisualizationTypesNum = 3;
		for (int32 Index = 0; Index < VisualizationTypesNum; ++Index)
		{
			FCells VisualizingStep = FCell::EmptyCells;
			FLinearColor Color = FLinearColor::White;
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

			constexpr float TextHeight = 263.f;
			constexpr float TextSize = 124.f;
			const FDisplayCellsParams DisplayParams{Color, TextHeight, TextSize, Symbol, Position};
			UCellsUtilsLibrary::DisplayCells(InOwner, VisualizingStep, DisplayParams);
		} // [Loopy visualization]
	}
#endif // [Editor]
}

// Enable or disable AI for this bot
void AMyAIController::SetAI(bool bShouldEnable)
{
	const APlayerCharacter* InOwner = GetPawn<APlayerCharacter>();
	const bool bWantsEnableDeadAI = !InOwner && bShouldEnable;
	if (bWantsEnableDeadAI
	    || !HasAuthority())
	{
		return;
	}

	Reset();

	UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	if (MoverComponent)
	{
		MoverComponent->SetBlockMovement(!bShouldEnable);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Listen game states to enable or disable AI
void AMyAIController::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	const bool bMatchStarted = CurrentGameState == ECurrentGameState::InGame;
	SetAI(bMatchStarted);
}

// Called when this level actor is destroyed on the Generated Map
void AMyAIController::OnPostRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	SetAI(false);
}

// Called when owner's movement is completed for the time step
void AMyAIController::OnOwnerMovementCompleted_Implementation(const FMoverTimeStep& TimeStep)
{
	UpdateAI();
}
