// Copyright (c) Yevhenii Selivanov.

#include "Controllers/BmrAIController.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "Components/BmrMoverComponent.h"
#include "DataAssets/BmrAIDataAsset.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "GameFramework/BmrCheatManager.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#endif

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrAIController)

// Sets default values for this character's properties
ABmrAIController::ABmrAIController()
{
	// Set this AI controller to don't call Tick()
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bAttachToPawn = true;
}

// Makes AI go toward specified destination cell
void ABmrAIController::MoveToCell(const FBmrCell& DestinationCell)
{
	ABmrPawn* InOwner = GetPawn<ABmrPawn>();
	const UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(InOwner);
	UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	if (!MapComponent
	    || !MoverComponent)
	{
		return;
	}

	if (!MoverComponent->IsBlockedMovement())
	{
		const FBmrCell& CurrentCell = MapComponent->GetCell();
		const bool bHasArrived = CurrentCell == DestinationCell;
		LastMoveToCell = bHasArrived ? FBmrCell::InvalidCell : DestinationCell;

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
			UBmrCellUtilsLibrary::ClearDisplayedCells(InOwner);
		}

		if (MapComponent->bShouldShowRenders)
		{
			static const FBmrDisplayCellsParams DisplayParams{FLinearColor::Gray, 255.f, 300.f, TEXT("x")};
			UBmrCellUtilsLibrary::DisplayCell(InOwner, DestinationCell, DisplayParams);
		}
	}
#endif
}

// Returns true if AI is enabled (move input is not ignored and cheat is not enabled)
bool ABmrAIController::IsAIEnabled() const
{
	const ABmrPawn* InOwner = GetPawn<ABmrPawn>();
	const UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	return MoverComponent
	       && !MoverComponent->IsBlockedMovement()
	       && UBmrCheatManager::CVarAISetEnabled.GetValueOnAnyThread();
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// This is called only in the gameplay before calling begin play
void ABmrAIController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register controller to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// Allows the controller to react on possessing the pawn
void ABmrAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABmrPawn* InOwner = Cast<ABmrPawn>(InPawn);
	if (!InPawn)
	{
		return;
	}

#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		if (!UBmrUnrealEdEngine::GOnAIUpdatedDelegate.IsBoundToObject(this))
		{
			UBmrUnrealEdEngine::GOnAIUpdatedDelegate.AddUObject(this, &ThisClass::UpdateAI);
		}

		// ! It's editor not Pie World, don't continue further runtime logic
		return;
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	if (GetPlayerState<ABmrPlayerState>() == nullptr)
	{
		// Spawn Player State for AI to replicate game-relevant info like scores, teams etc
		InitPlayerState();
		ABmrPlayerState* NewPlayerState = GetPlayerState<ABmrPlayerState>();
		checkf(NewPlayerState, TEXT("ERROR: [%i] %s:\n'NewPlayerState' was not spawned!"), __LINE__, *FString(__FUNCTION__));
		InOwner->SetPlayerState(NewPlayerState);

		// Update default nickname for AI
		NewPlayerState->SetDefaultPlayerName();
	}

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// Notify host about bot possession
	UBmrGameplayMessageSubsystem::Get().ReadyHandler.Broadcast_OnPawnPossessed(*InOwner);

	UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(InOwner);
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);

	// Subscribe to movement completion to trigger AI updates
	UBmrMoverComponent* MoverComponent = InOwner->GetMoverComponent();
	checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
	MoverComponent->OnPostSimulationTick.AddUniqueDynamic(this, &ThisClass::OnOwnerMovementCompleted);

	const bool bMatchStarted = ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::InGame);
	SetAI(bMatchStarted);
}

// Allows the controller to react on unpossessing the pawn
void ABmrAIController::OnUnPossess()
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		UBmrUnrealEdEngine::GOnAIUpdatedDelegate.RemoveAll(this);
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	SetAI(false);

	if (const ABmrPawn* InOwner = GetPawn<ABmrPawn>())
	{
		UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(InOwner);
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
		MapComponent->OnPostRemovedFromLevel.RemoveAll(this);

		UBmrMoverComponent* MoverComponent = InOwner->GetMoverComponent();
		checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
		MoverComponent->OnPostSimulationTick.AddUniqueDynamic(this, &ThisClass::OnOwnerMovementCompleted);
	}

	Super::OnUnPossess();
}

// Stops running to target
void ABmrAIController::Reset()
{
	// Abort current movement task
	Super::Reset();

	// Reset target location
	LastMoveToCell = FBmrCell::InvalidCell;

	const ABmrPawn* InOwner = GetPawn<ABmrPawn>();
	UBmrMoverComponent* MoverComponent = InOwner ? InOwner->GetMoverComponent() : nullptr;
	if (MoverComponent)
	{
		MoverComponent->RequestMoveByIntent(FVector::ZeroVector);
	}
}

// The main AI logic
void ABmrAIController::UpdateAI()
{
	ABmrPawn* InOwner = GetPawn<ABmrPawn>();
	const UBmrMapComponent* MapComponent = InOwner ? UBmrMapComponent::GetMapComponent(InOwner) : nullptr;
	if (!MapComponent
	    || !IsAIEnabled())
	{
		return;
	}

	// Throttle AI updates to match desired tick rate
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeSinceLastUpdate = CurrentTime - LastAIUpdateTime;
	if (TimeSinceLastUpdate < UBmrGameStateDataAsset::Get().GetTickInterval())
	{
		return;
	}
	LastAIUpdateTime = CurrentTime;

	// Stop movement if arrived at destination
	if (LastMoveToCell.IsValid()
	    && MapComponent->GetCell() == LastMoveToCell)
	{
		Reset();
		// Fall through to choose new destination
	}

	const UBmrAIDataAsset& AIDataAsset = UBmrAIDataAsset::Get();

	if (UUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UBmrCellUtilsLibrary::ClearDisplayedCells(InOwner);
		LastMoveToCell = FBmrCell::InvalidCell;
	}

	// ----- Part 0: Before iterations -----

	// Set the START cell searching bot location
	const FBmrCell& F0 = MapComponent->GetCell();

	// Searching 'SAFE NEIGHBORS'
	static constexpr int32 MaxInteger = TNumericLimits<int32>::Max();
	FBmrCells Free;
	uint8 bIsDangerous;
	for (bIsDangerous = 0; bIsDangerous <= 1; ++bIsDangerous) // two searches (safe and free)
	{
		Free = UBmrCellUtilsLibrary::GetCellsAround(F0, bIsDangerous ? EPathType::Free : EPathType::Safe, MaxInteger);
		if (!bIsDangerous && Free.Num() > 0)
		{
			// Remove this cell from array
			bIsDangerous = !Free.Remove(F0); // if it can't be removed - the bot is standing in the explosion
			break;
		}
	}

	// Is there an powerup nearby?
	if (bIsDangerous == false)
	{
		const FBmrCells PowerupsFromF0 = UBmrCellUtilsLibrary::GetCellsAroundWithActors(F0, EPathType::Safe, AIDataAsset.GetPowerupSearchRadius(), TO_FLAG(EAT::Powerup));
		if (!PowerupsFromF0.IsEmpty())
		{
			MoveToCell(FBmrCell::GetFirstCellInSet(PowerupsFromF0));
			return;
		}
	}
	// ----- Part 1: Cells iteration -----

	FBmrCells AllCrossways; //  cells of all crossways
	FBmrCells SecureCrossways; // crossways without players
	FBmrCells FoundPowerups;
	bool bIsPowerupInDirect = false;

	for (auto F = Free.CreateIterator(); F; ++F)
	{
		if (bIsDangerous // is not dangerous situation
		    && FBmrCell::Distance<float>(F0, *F) > AIDataAsset.GetNearDangerousRadius())
		{
			F.RemoveCurrent(); // removing distant cells
			continue;
		}

		const FBmrCells ThisCrossway = UBmrCellUtilsLibrary::GetCellsAround(*F, EPathType::Safe, AIDataAsset.GetCrosswaySearchRadius());
		FBmrCells Way = Free; // Way = Safe / (Free + F0)
		Way.Emplace(F0); // Way = Free + F0
		Way = ThisCrossway.Difference(Way); // Way = Safe / Way

		if (Way.Num() > 0) // Are there any cells?
		{
			// Finding crossways
			AllCrossways.Emplace(*F); // is the crossway
			Way = UBmrCellUtilsLibrary::FilterCellsByActors(ThisCrossway, TO_FLAG(EAT::Player));
			Way.Remove(MapComponent->GetCell());
			if (Way.Num() == 0)
			{
				SecureCrossways.Emplace(*F);
			}

			// Finding Powerups
			FBmrCells PowerupsAround = UBmrCellUtilsLibrary::FilterCellsByActors(ThisCrossway, TO_FLAG(EAT::Powerup));
			if (PowerupsAround.Num() > 0) // Is there Powerups in this crossway?
			{
				PowerupsAround = PowerupsAround.Intersect(Free); // PowerupsAround = PowerupsAround ∪ Free
				if (PowerupsAround.Num() > 0) // Is there direct Powerups in this crossway?
				{
					if (bIsPowerupInDirect == false) // is the first found direct Powerup
					{
						bIsPowerupInDirect = true;
						FoundPowerups.Empty(); // clear all previously found corner Powerups
					}
					FoundPowerups = FoundPowerups.Union(PowerupsAround); // Add found direct Powerups
				} // Powerup around the corner
				else if (bIsPowerupInDirect == false) // Need corner Powerup?
				{
					FoundPowerups.Emplace(*F); // Add found corner Powerup
				}
			} // [has Powerups]
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

	FBmrCells Filtered = FoundPowerups.Num() > 0 ? FoundPowerups : Free; // selected cells
	bool bIsFilteringFailed = false;
	static constexpr int32 FilteringStepsNum = 4;
	for (int32 Index = 0; Index < FilteringStepsNum; ++Index)
	{
		FBmrCells FilteringStep;
		switch (Index)
		{
			case 0: // All crossways: Filtered ∪ AllCrossways
				FilteringStep = Filtered.Intersect(AllCrossways);
				break;
			case 1: // Without players
				FilteringStep = UBmrCellUtilsLibrary::GetCellsAround(F0, EPathType::Secure, MaxInteger);
				FilteringStep = Filtered.Intersect(FilteringStep);
				break;
			case 2: // Without crossways with another players
				FilteringStep = Filtered.Intersect(SecureCrossways);
				break;
			case 3: // Only nearest cells (length <= near radius)
				for (const FBmrCell& It : Filtered)
				{
					if (FBmrCell::Distance<float>(F0, It) <= AIDataAsset.GetNearFilterRadius())
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
	    && !bIsPowerupInDirect) // was not found direct Powerups
	{
		const float Fire = UBmrPowerupsAttributeSet::Get(InOwner).GetPowerup_Fire();
		FBmrCells BoxesAndPlayers = UBmrCellUtilsLibrary::GetCellsAroundWithActors(F0, EPathType::Explosion, Fire, TO_FLAG(EAT::Box | EAT::Player));
		BoxesAndPlayers.Remove(MapComponent->GetCell());
		if (BoxesAndPlayers.Num() > 0) // Are bombs or players in own bomb radius
		{
			InOwner->SpawnBomb();
			Free.Empty(); // Delete all cells to make new choice

#if WITH_EDITOR // [Editor]
			if (MapComponent->bShouldShowRenders)
			{
				static const FBmrDisplayCellsParams DisplayParams{FLinearColor::Red, 261.F, 95.F, TEXT("Attack")};
				UBmrCellUtilsLibrary::DisplayCell(InOwner, F0, DisplayParams);
			}
#endif // [Editor]
		}
	}

	// ----- Part 3: Making choice-----

	if (Free.Contains(LastMoveToCell))
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
			FBmrCells VisualizingStep = FBmrCell::EmptyCells;
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
			const FBmrDisplayCellsParams DisplayParams{Color, TextHeight, TextSize, Symbol, Position};
			UBmrCellUtilsLibrary::DisplayCells(InOwner, VisualizingStep, DisplayParams);
		} // [Loopy visualization]
	}
#endif // [Editor]
}

// Enable or disable AI for this bot
void ABmrAIController::SetAI(bool bShouldEnable)
{
	const ABmrPawn* InOwner = GetPawn<ABmrPawn>();
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
void ABmrAIController::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const bool bMatchStarted = Payload.InstigatorTags.HasTag(FBmrGameStateTag::InGame);
	SetAI(bMatchStarted);
}

// Called when this level actor is destroyed on the Generated Map
void ABmrAIController::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* MapComponent, UObject* DestroyCauser)
{
	SetAI(false);
}

// Called when owner's movement is completed for the time step
void ABmrAIController::OnOwnerMovementCompleted_Implementation(const FMoverTimeStep& TimeStep)
{
	UpdateAI();
}
