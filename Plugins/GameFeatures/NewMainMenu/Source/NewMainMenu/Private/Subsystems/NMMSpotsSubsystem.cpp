// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMSpotsSubsystem.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "DalRegistrySubsystem.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"

// Bomber
#include "DataRegistries/BmrCinematicRow.h"
#include "GameFramework/BmrGameState.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMSpotsSubsystem)

UNMMSpotsSubsystem& UNMMSpotsSubsystem::Get(const UObject* OptionalWorldContext)
{
	UNMMSpotsSubsystem* ThisSubsystem = UNMMUtils::GetSpotsSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'NMMSpotsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns true if any Main-Menu spot is fully initialized: spawned on the level and finished loading its Master Sequence
bool UNMMSpotsSubsystem::IsActiveMenuSpotReady() const
{
	const UNMMSpotComponent* ActiveSpot = GetCurrentSpot();
	return ActiveSpot && ActiveSpot->GetMasterPlayer() != nullptr;
}

// Add new Main-Menu spot, so it can be obtained by other objects
void UNMMSpotsSubsystem::AddNewMainMenuSpot(UNMMSpotComponent* NewMainMenuSpotComponent)
{
	if (!ensureMsgf(NewMainMenuSpotComponent, TEXT("%s: 'NewMainMenuSpotComponent' is null"), *FString(__FUNCTION__)))
	{
		return;
	}

	MainMenuSpots.AddUnique(NewMainMenuSpotComponent);

	// Init the late spot immediately, otherwise wait until cinematics load completes
	if (NewMainMenuSpotComponent->GetCinematicRow().IsValid())
	{
		NewMainMenuSpotComponent->InitMasterSequencePlayer();
	}
}

// Reinitializes cinematic data for all spots from Data Registry
void UNMMSpotsSubsystem::ReinitializeAllSpots()
{
	for (UNMMSpotComponent* SpotComponent : MainMenuSpots)
	{
		if (SpotComponent)
		{
			SpotComponent->ReinitializeCinematicData();
		}
	}

	UDalRegistrySubsystem::Get().TryLoad(this);
}

// Activates local player's spot once all spots finished loading, nothing otherwise
void UNMMSpotsSubsystem::TryActivateMenuSpot()
{
	// Once menu owns selection, resolve its spot as soon as it loads so one stuck sibling never blocks re-activation, only first resolution waits for all spots to pick highest
	const bool bHasMenuSelection = ActiveSpotPriority != INDEX_NONE;
	if (!bHasMenuSelection && !AreAllSpotsLoaded())
	{
		// Spots still loading, wait for last one
		return;
	}

	UNMMSpotComponent* ActiveSpot = FindLocalPlayerSpot();
	if (!ActiveSpot)
	{
		// Local player's spot not loaded yet, defer until it does
		return;
	}

	ActiveSpotPriority = ActiveSpot->GetCinematicRow().Priority;
	// Reset player to resolved spot's character, so match's in-game per-player character never lingers in menu when re-activated without state change
	ActiveSpot->ApplyMeshOnPlayer();

	// Apply cinematic state that was deferred during async load
	const FNmmStateTag CurrentState = UNMMBaseSubsystem::Get().GetCurrentMenuState();
	ActiveSpot->SetCinematicByState(CurrentState);

	NotifyActiveMenuSpotReady(ActiveSpot);
}

// Returns true if all spots with cinematic data have finished loading their Master Sequences
bool UNMMSpotsSubsystem::AreAllSpotsLoaded() const
{
	for (const UNMMSpotComponent* SpotIt : MainMenuSpots)
	{
		// Spot has cinematic data but hasn't finished loading its sequence yet
		if (SpotIt
		    && !SpotIt->GetCinematicRow().IsEmpty()
		    && !SpotIt->GetMasterPlayer())
		{
			return false;
		}
	}

	return !MainMenuSpots.IsEmpty();
}

// Returns deterministic highest-priority spot on first resolution, then spot matching active spot priority once it is set, null while that spot is still loading
UNMMSpotComponent* UNMMSpotsSubsystem::FindLocalPlayerSpot() const
{
	const bool bHasMenuSelection = ActiveSpotPriority != INDEX_NONE;
	UNMMSpotComponent* HighestPrioritySpot = nullptr;
	for (UNMMSpotComponent* SpotIt : MainMenuSpots)
	{
		if (!SpotIt || !SpotIt->GetMasterPlayer())
		{
			// Skip spots that have not finished loading their Master Sequence
			continue;
		}

		if (bHasMenuSelection
		    && SpotIt->GetCinematicRow().Priority == ActiveSpotPriority)
		{
			// Spot matching active spot priority loaded: resolve to it, not to chosen mesh data, which a match overwrites with the in-game per-player character
			return SpotIt;
		}

		const bool bIsHigherPriority = !HighestPrioritySpot || SpotIt->GetCinematicRow().Priority > HighestPrioritySpot->GetCinematicRow().Priority;
		if (bIsHigherPriority)
		{
			HighestPrioritySpot = SpotIt;
		}
	}

	if (bHasMenuSelection)
	{
		// Selection exists but its spot is still loading: defer so non-selected spot never activates and clobbers selection
		return nullptr;
	}

	return HighestPrioritySpot;
}

// Notifies listeners that active menu spot is ready
void UNMMSpotsSubsystem::NotifyActiveMenuSpotReady(UNMMSpotComponent* Spot)
{
	OnActiveMenuSpotReady.Broadcast(Spot);
}

// Removes Main-Menu spot if should not be available by other objects anymore
void UNMMSpotsSubsystem::RemoveMainMenuSpot(UNMMSpotComponent* MainMenuSpotComponent)
{
	if (ensureMsgf(MainMenuSpotComponent, TEXT("%s: 'MainMenuSpotComponent' is null"), *FString(__FUNCTION__)))
	{
		MainMenuSpots.RemoveSwap(MainMenuSpotComponent);
	}
}

// Returns currently selected Main-Menu spot
UNMMSpotComponent* UNMMSpotsSubsystem::GetCurrentSpot() const
{
	if (ActiveSpotPriority == INDEX_NONE)
	{
		return nullptr;
	}

	for (UNMMSpotComponent* MainMenuSpotComponent : MainMenuSpots)
	{
		if (MainMenuSpotComponent && MainMenuSpotComponent->GetCinematicRow().Priority == ActiveSpotPriority)
		{
			return MainMenuSpotComponent;
		}
	}

	return nullptr;
}

// Returns all valid Main-Menu spots sorted by priority
void UNMMSpotsSubsystem::GetMainMenuSpots(TArray<UNMMSpotComponent*>& OutSpots) const
{
	for (UNMMSpotComponent* MainMenuSpotComponent : MainMenuSpots)
	{
		if (MainMenuSpotComponent
		    && MainMenuSpotComponent->GetCinematicRow().IsValid())
		{
			OutSpots.AddUnique(MainMenuSpotComponent);
		}
	}

	// Sort the array based on the Priority, higher priority first
	OutSpots.Sort([](const UNMMSpotComponent& A, const UNMMSpotComponent& B)
	{
		return A.GetCinematicRow().Priority > B.GetCinematicRow().Priority;
	});
}

// Returns next or previous Main-Menu spot by given incrementer
UNMMSpotComponent* UNMMSpotsSubsystem::GetNextSpot(int32 Incrementer) const
{
	TArray<UNMMSpotComponent*> CurrentLevelTypeSpots;
	GetMainMenuSpots(/*out*/ CurrentLevelTypeSpots);

	// Extract the priorities, so we can track the bounds
	TArray<int32> SpotPriorities;
	for (const UNMMSpotComponent* SpotIt : CurrentLevelTypeSpots)
	{
		SpotPriorities.AddUnique(SpotIt->GetCinematicRow().Priority);
	}

	const bool bFoundActivePriority = SpotPriorities.Contains(ActiveSpotPriority);
	if (!ensureMsgf(bFoundActivePriority, TEXT("%s: 'ActiveSpotPriority' is not found in the 'SpotPriorities'"), *FString(__FUNCTION__)))
	{
		// Most likely the level is switched that could be not supported yet
		return nullptr;
	}

	// Find the new index based on the incrementer
	// If there is no next spot in array, it will take the first one with its index and vise versa for decrementing
	const int32 ActiveSpotPosition = SpotPriorities.IndexOfByKey(ActiveSpotPriority);
	const int32 NewSpotIndex = (ActiveSpotPosition + Incrementer + SpotPriorities.Num()) % SpotPriorities.Num();
	checkf(CurrentLevelTypeSpots.IsValidIndex(NewSpotIndex), TEXT("ERROR: [%i] %s:\n'CurrentLevelTypeSpots array has to have NewSpotIndex since it's the same size as SpotPriorities array!"), __LINE__, *FString(__FUNCTION__));
	return CurrentLevelTypeSpots[NewSpotIndex];
}

// Goes to another Spot to show another player character on current level
UNMMSpotComponent* UNMMSpotsSubsystem::MoveMainMenuSpot(int32 Incrementer)
{
	UNMMSpotComponent* NextMainMenuSpot = GetNextSpot(Incrementer);
	if (!ensureMsgf(NextMainMenuSpot, TEXT("ASSERT: [%i] %s:\n'NextMainMenuSpot' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	// Set the new active spot priority
	ActiveSpotPriority = NextMainMenuSpot->GetCinematicRow().Priority;
	LastMoveSpotDirection = Incrementer;

	// If transition happened in opened menu, change the internal state
	if (ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::Menu))
	{
		// If instant, then switch to the next spot, it will possess the camera and start playing its cinematic
		// Otherwise start transition to the next spot
		const bool bInstant = UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled();
		UNMMBaseSubsystem::Get().SetNewMainMenuState(bInstant ? FNmmStateTag::Idle : FNmmStateTag::Transition);
	}
	else // In-game
	{
		// Thw spot switch happened during the game, just update the mesh hiddenly
		NextMainMenuSpot->ApplyMeshOnPlayer();
	}

	return NextMainMenuSpot;
}

// Is code alternative to MoveMainMenuSpot, but allows to use custom predicate
UNMMSpotComponent* UNMMSpotsSubsystem::MoveMainMenuSpotByPredicate(int32 Incrementer, const TFunctionRef<bool(UNMMSpotComponent*)>& Predicate)
{
	const int32 FinalIncrementer = [&]() -> int32
	{
		TArray<UNMMSpotComponent*> AllSpots;
		GetMainMenuSpots(AllSpots);

		if (AllSpots.IsEmpty())
		{
			return 0;
		}

		UNMMSpotComponent* Spot = nullptr;
		int32 Attempts = 0;
		const int32 MaxAttempts = AllSpots.Num();
		int32 NewIncrementer = Incrementer;

		while (Attempts < MaxAttempts)
		{
			Spot = GetNextSpot(NewIncrementer);
			if (Spot && Predicate(Spot))
			{
				return NewIncrementer;
			}

			NewIncrementer += Incrementer;
			Attempts++;
		}

		return 0;
	}();

	// -1 or 1 to move left or right, 0 if no spot found
	if (FinalIncrementer == 0)
	{
		return nullptr;
	}

	// Move to the new spot
	return MoveMainMenuSpot(FinalIncrementer);
}

// Attempts to switch to the active menu spot if current slot is not available for any reason
void UNMMSpotsSubsystem::HandleUnavailableMenuSpot()
{
	if (UNMMSpotComponent* CurrentSpot = GetCurrentSpot())
	{
		const UNMMCameraSubsystem* CameraSubsystem = UNMMUtils::GetCameraSubsystem();
		const ENMMCameraRailTransitionState RailState = CameraSubsystem ? CameraSubsystem->GetCurrentCameraRailTransitionState() : ENMMCameraRailTransitionState::None;
		if (RailState == ENMMCameraRailTransitionState::BeginTransition
		    || RailState == ENMMCameraRailTransitionState::HalfwayTransition)
		{
			// Match started while transition is happening between spots, apply mesh early as interruption scenario
			CurrentSpot->ApplyMeshOnPlayer();
		}

		if (CurrentSpot->IsSpotAvailable())
		{
			// Current spot available, keep it
			return;
		}
		// Fallthrough to move to last available spot
	}

	// Current is inactive (hidden), likely it's locked by different systems
	// Switch to any previous spot that is active
	constexpr int32 BackwardDir = -1;
	MoveMainMenuSpotByPredicate(BackwardDir, [](const UNMMSpotComponent* Spot)
	{
		return Spot->IsSpotAvailable();
	});
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Subscribes to menu state and game state events
void UNMMSpotsSubsystem::OnGameFeatureInitialize_Implementation()
{
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(NmmGameplayTags::Event::MenuStateChanged, this, &ThisClass::OnNewMainMenuStateChanged);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	UDalRegistrySubsystem::Get().BindAndLoad<FBmrCinematicRow>(this, &ThisClass::OnCinematicRowsChanged);
}

// Clears all transient data contained in this subsystem
void UNMMSpotsSubsystem::OnGameFeatureDeinitialize_Implementation()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->UnbindFromDataRegistryLoad(this);
	}

	MainMenuSpots.Empty();
	ActiveSpotPriority = INDEX_NONE;
	LastMoveSpotDirection = 0;
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the Main Menu state was changed
void UNMMSpotsSubsystem::OnNewMainMenuStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const FNmmStateTag NewState(Payload.InstigatorTags.First());
	if (NewState == FNmmStateTag::None
	    || NewState == FNmmStateTag::BasicMenu)
	{
		LastMoveSpotDirection = 0;
	}
	else if ((FNmmStateTag::Idle | FNmmStateTag::Cinematic).HasTag(NewState))
	{
		UNMMSpotComponent* ActiveSpot = GetCurrentSpot();
		if (ActiveSpot && ActiveSpot->GetMasterPlayer())
		{
			ActiveSpot->SetCinematicByState(NewState);
			NotifyActiveMenuSpotReady(ActiveSpot);
		}
	}
}

// Called when the current game state was changed
void UNMMSpotsSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		HandleUnavailableMenuSpot();
	}
}

// Called when cinematic Data Registry rows change and all their soft references finish async loading
void UNMMSpotsSubsystem::OnCinematicRowsChanged_Implementation()
{
	bool bAnySpotHasValidRow = false;
	for (UNMMSpotComponent* SpotIt : MainMenuSpots)
	{
		if (SpotIt)
		{
			SpotIt->ReinitializeCinematicData();
			bAnySpotHasValidRow |= SpotIt->GetCinematicRow().IsValid();
		}
	}

	const bool bHasRows = FBmrCinematicRow::HasRows();

	UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();
	const FNmmStateTag CurrentState = BaseSubsystem.GetCurrentMenuState();

	if (bHasRows
	    && bAnySpotHasValidRow
	    && CurrentState == FNmmStateTag::BasicMenu)
	{
		// Cinematics rows injected by map GFP and spots are ready, activate cinematic lobby
		BaseSubsystem.SetNewMainMenuState(FNmmStateTag::Idle);
	}
	else if (!bHasRows
	         && CurrentState != FNmmStateTag::None
	         && CurrentState != FNmmStateTag::BasicMenu)
	{
		// DR emptied at runtime (map GFP unloaded), fall back to basic menu
		BaseSubsystem.SetNewMainMenuState(FNmmStateTag::BasicMenu);
	}

	for (UNMMSpotComponent* SpotIt : MainMenuSpots)
	{
		if (SpotIt)
		{
			SpotIt->InitMasterSequencePlayer();
		}
	}
}
