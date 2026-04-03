// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMBaseSubsystem.h"

// NMM
#include "NMMUtils.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Structures/BmrCinematicRow.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMBaseSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNMMBaseSubsystem& UNMMBaseSubsystem::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	UNMMBaseSubsystem* ThisSubsystem = UNMMUtils::GetBaseSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

/*********************************************************************************************
 * New Main Menu State
 ********************************************************************************************* */

// Applies the new state of New Main Menu game feature
void UNMMBaseSubsystem::SetNewMainMenuState(ENMMState NewState)
{
	const ENMMState PreviousState = CurrentMenuState;
	CurrentMenuState = NewState;

	OnMainMenuStateChanged.Broadcast(NewState, PreviousState);
}

// Returns true if DR_Cinematics Data Registry has any cached rows
bool UNMMBaseSubsystem::HasCinematicRows()
{
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	const UDataRegistry* Registry = DRSubsystem ? DRSubsystem->GetRegistryForType(FDataRegistryType(FBmrCinematicRow::CinematicsRegistryTypeName)) : nullptr;
	if (!ensureMsgf(Registry, TEXT("ASSERT: [%i] %hs:\n'DR_Cinematics' Data Registry is not found, make sure it is created with '%s' type"), __LINE__, __FUNCTION__, *FBmrCinematicRow::CinematicsRegistryTypeName.ToString()))
	{
		return false;
	}

	bool bHasRows = false;
	Registry->ForEachCachedItem<FBmrCinematicRow>(TEXT("HasCinematicRows"), [&bHasRows](const FName& /*Name*/, const FBmrCinematicRow& /*Row*/)
	{
		bHasRows = true;
	});

	return bHasRows;
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Is called when the world is initialized
void UNMMBaseSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	// Listen for DR_Cinematics changes to transition between BasicMenu and Idle when map MGF injects or removes rows
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	UDataRegistry* Registry = DRSubsystem ? DRSubsystem->GetRegistryForType(FDataRegistryType(FBmrCinematicRow::CinematicsRegistryTypeName)) : nullptr;
	if (!ensureMsgf(Registry, TEXT("ASSERT: [%i] %hs:\n'DR_Cinematics' Data Registry is not found, make sure it is created with '%s' type"), __LINE__, __FUNCTION__, *FBmrCinematicRow::CinematicsRegistryTypeName.ToString()))
	{
		return;
	}

	FDataRegistryCacheVersionCallback& CacheDelegate = Registry->OnCacheVersionInvalidated();
	if (!CacheDelegate.IsBoundToObject(this))
	{
		CacheDelegate.AddUObject(this, &ThisClass::OnCinematicsRegistryChanged);
	}

	// Listen for spot readiness to re-evaluate state when spots finish loading after DR rows already arrived
	UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	SpotsSubsystem.OnActiveMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnActiveMenuSpotReady);
}

// Clears all transient data contained in this subsystem
void UNMMBaseSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	Super::OnWorldEndPlay(InWorld);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the current game state was changed, handles Main Menu states accordingly
void UNMMBaseSubsystem::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		// Always start in BasicMenu, DR change callback will transition to Idle when cinematics are available
		// Skip BasicMenu when cinematics are already loaded (e.g. returning from game to menu)
		const ENMMState MenuState = HasCinematicRows() ? ENMMState::Idle : ENMMState::BasicMenu;
		SetNewMainMenuState(MenuState);
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		// Player left the Main Menu
		SetNewMainMenuState(ENMMState::None);
	}
}

// Called when DR_Cinematics Data Registry cache version changes (rows injected or removed)
void UNMMBaseSubsystem::OnCinematicsRegistryChanged_Implementation(UDataRegistry* CinematicsDataRegistry)
{
	const UWorld* World = GetWorld();
	if (!World
	    || World->bIsTearingDown
	    || !CinematicsDataRegistry
	    || CinematicsDataRegistry->GetLowestAvailability() == EDataRegistryAvailability::DoesNotExist)
	{
		return;
	}

	const bool bHasRows = HasCinematicRows();
	const UNMMSpotsSubsystem* SpotsSubsystem = UNMMUtils::GetSpotsSubsystem();
	const bool bSpotsReady = SpotsSubsystem && SpotsSubsystem->IsActiveMenuSpotReady();

	if (bHasRows && bSpotsReady
	    && CurrentMenuState == ENMMState::BasicMenu)
	{
		// Cinematics rows injected by map MGF and spots are ready, activate cinematic lobby
		SetNewMainMenuState(ENMMState::Idle);
	}
	else if (!bHasRows
	         && CurrentMenuState != ENMMState::None
	         && CurrentMenuState != ENMMState::BasicMenu)
	{
		// DR emptied at runtime (map MGF unloaded), fall back to basic menu
		SetNewMainMenuState(ENMMState::BasicMenu);
	}
}

// Called when a cinematic spot finished loading, re-evaluates whether to transition from BasicMenu to Idle
void UNMMBaseSubsystem::OnActiveMenuSpotReady_Implementation(UNMMSpotComponent* /*MainMenuSpotComponent*/)
{
	if (CurrentMenuState != ENMMState::BasicMenu
	    || !HasCinematicRows())
	{
		return;
	}

	// DR rows are present and spots just became ready, activate cinematic lobby
	SetNewMainMenuState(ENMMState::Idle);
}
