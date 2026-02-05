// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrGameState.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Components/BmrGameDifficultyManagerComponent.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "DataAssets/BmrModularGameFeatureSettings.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/StateTreeComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameState)

// Default constructor
ABmrGameState::ABmrGameState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GameDifficultyManager = CreateDefaultSubobject<UBmrGameDifficultyManagerComponent>(TEXT("GameFeaturesManager"));
	GameStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("GameStateTree"));
	GameStateTreeComponent->SetStartLogicAutomatically(false);
}

// Returns the current game state, it will crash if can't be obtained, should be used only when the game is running
ABmrGameState& ABmrGameState::Get()
{
	ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState();
	checkf(MyGameState, TEXT("ERROR: [%i] %s:\n'MyGameState' is null!"), __LINE__, *FString(__FUNCTION__));
	return *MyGameState;
}

/*********************************************************************************************
 * Game State Tree
 ********************************************************************************************* */

// Returns the Game State that is currently applied
EBmrCurrentGameState ABmrGameState::GetCurrentGameState()
{
	if (const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState())
	{
		return MyGameState->ReplicatedGameState;
	}
	return EBmrCurrentGameState::None;
}

// Returns the Game State that was applied before the current one
EBmrCurrentGameState ABmrGameState::GetPreviousGameState()
{
	if (const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState())
	{
		return MyGameState->LocalPreviousGameState;
	}
	return EBmrCurrentGameState::None;
}

// Returns true if the game state State Tree can be started, is false when in Render Movie cinematic mode
bool ABmrGameState::CanStartGameStateTree() const
{
	const UWorld* World = GetWorld();
	const APlayerController* LocalPC = World ? World->GetFirstPlayerController() : nullptr;
	const bool bCinematicMode = LocalPC && LocalPC->bCinematicMode;
	return !bCinematicMode
	       && GameStateTreeComponent
	       && !GameStateTreeComponent->IsRunning();
}

// Initializes the State Tree, that is used to manage the overall game state
void ABmrGameState::StartGameStateTree()
{
	if (!HasAuthority()
	    || !CanStartGameStateTree())
	{
		UE_LOG(LogBomber, Verbose, TEXT("[%i] %hs:\nCannot start GameStateTree, authority: %i, can start: %i"), __LINE__, __FUNCTION__, HasAuthority(), CanStartGameStateTree());
		return;
	}

	UStateTree* GameStateTreeAsset = UBmrGameStateDataAsset::Get().GetGameStateTreeAsset();
	if (!ensureMsgf(GameStateTreeAsset, TEXT("ASSERT: [%i] %hs:\n'GameStateTreeAsset' is not set!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	checkf(GameStateTreeComponent, TEXT("ERROR: [%i] %hs:\n'GameStateTreeComponent' is null!"), __LINE__, __FUNCTION__);
	GameStateTreeComponent->SetStateTree(GameStateTreeAsset);
	GameStateTreeComponent->StartLogic();
}

// Stops the State Tree that manages the overall game state
void ABmrGameState::StopGameStateTree()
{
	if (!HasAuthority())
	{
		return;
	}

	checkf(GameStateTreeComponent, TEXT("ERROR: [%i] %hs:\n'GameStateTreeComponent' is null!"), __LINE__, __FUNCTION__);
	if (!GameStateTreeComponent->IsRunning())
	{
		return;
	}

	static const FString Reason{__FUNCTION__};
	GameStateTreeComponent->StopLogic(Reason);
}

// Is the only proper way to change the game state, called by the State Tree on the server
void ABmrGameState::SetGameState(EBmrCurrentGameState NewGameState)
{
	if (!HasAuthority()
	    || ReplicatedGameState == NewGameState)
	{
		return;
	}

	ReplicatedGameState = NewGameState;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedGameState, this);

	ApplyGameState();
}

// Updates current game state
void ABmrGameState::ApplyGameState()
{
	TRACE_BOOKMARK(TEXT("%s"), *UEnum::GetValueAsString(ReplicatedGameState));

	// Notify listeners via Gameplay Message Router
	FGameplayEventData Payload;
	Payload.EventTag = BmrGameplayTags::Event::GameState_Changed;
	UBmrGameplayMessageSubsystem::BroadcastMessage(Payload);

	// Update previous state after broadcasting, so listeners see the correct previous state during the event
	LocalPreviousGameState = ReplicatedGameState;
}

// Called on the AMyGameState::CurrentGameState property updating.
void ABmrGameState::OnRep_CurrentGameState()
{
	if (!UBmrGameplayMessageSubsystem::Get().ReadyHandler.IsReady(UBmrBlueprintFunctionLibrary::GetLocalPawn()))
	{
		// Pawn might not be ready on client, apply replicated game state now (otherwise it would be deferred until pawn is ready)
		return;
	}

	ApplyGameState();
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void ABmrGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedGameState, Params);
}

// This is called only in the gameplay before calling begin play
void ABmrGameState::PostInitializeComponents()
{
	// Register it to let modular feature to be dynamically added
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	Super::PostInitializeComponents();
}

// Called when the game starts
void ABmrGameState::BeginPlay()
{
	Super::BeginPlay();

	StartGameStateTree();

	UGameplayUtilsLibrary::SetGameFeaturesEnabled(true, UBmrModularGameFeatureSettings::Get().GetModularGameFeatures());

	BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
}

// Overridable function called whenever this actor is being removed from a level
void ABmrGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UGameplayUtilsLibrary::SetGameFeaturesEnabled(false, UBmrModularGameFeatureSettings::Get().GetModularGameFeatures());
}

// Called when the local player character is spawned, possessed, and replicated
void ABmrGameState::OnLocalPawnReady_Implementation(const FGameplayEventData& Payload)
{
	if (LocalPreviousGameState == ReplicatedGameState)
	{
		// Game state was already applied or nothing to apply
		return;
	}

	// On client, OnRep_CurrentGameState might have skipped applying if the pawn was not ready at that time,
	// so immediately apply any pending replicated game state now that the pawn is ready
	ApplyGameState();
}