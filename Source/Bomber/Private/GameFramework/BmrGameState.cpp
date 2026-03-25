// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrGameState.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrGameDifficultyManagerComponent.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/StateTreeComponent.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameState)

// Default constructor
ABmrGameState::ABmrGameState()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GameDifficultyManager = CreateDefaultSubobject<UBmrGameDifficultyManagerComponent>(TEXT("GameDifficultyManager"));
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

// Registers to listen for GameState tag changes on ASC, on both server and client
void ABmrGameState::BindOnGameStateTagChanged()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	ASC->RegisterGenericGameplayTagEvent().AddWeakLambda(this, [this](const FGameplayTag Tag, int32 NewCount)
	{
		if (NewCount > 0 // Added
		    && Tag.MatchesTag(FBmrGameStateTag::ParentTag) // Child
		    && Tag != FBmrGameStateTag::ParentTag) // Not parent itself
		{
			BroadcastGameStateChanged();
		}
	});
}

// Broadcasts the GameState_Changed event via Gameplay Message Router
void ABmrGameState::BroadcastGameStateChanged()
{
	if (!HasAuthority()
	    && !UBmrBlueprintFunctionLibrary::IsLocalPawnReady())
	{
		// Client is not ready yet, do not broadcast pawn is loaded
		return;
	}

	FGameplayTagContainer OwnedTags;
	GetOwnedGameplayTags(OwnedTags);

	FGameplayEventData Payload;
	Payload.EventTag = BmrGameplayTags::Event::GameState_Changed;
	Payload.InstigatorTags = OwnedTags.Filter(FBmrGameStateTag::ParentTag.GetSingleTagContainer());
	UBmrGameplayMessageSubsystem::BroadcastMessage(Payload);

	TRACE_BOOKMARK(TEXT("%s"), *Payload.InstigatorTags.ToStringSimple());
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns the Ability System Component from the Generated Map
UAbilitySystemComponent* ABmrGameState::GetAbilitySystemComponent() const
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	return GeneratedMap ? GeneratedMap->GetAbilitySystemComponent() : nullptr;
}

UAbilitySystemComponent& ABmrGameState::GetAbilitySystemComponentChecked() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'ASC' is null!"), __LINE__, __FUNCTION__);
	return *ASC;
}

// Returns the gameplay tags owned by this actor via IGameplayTagAssetInterface
void ABmrGameState::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetOwnedGameplayTags(TagContainer);
	}
}

// This is called only in the gameplay before calling begin play
void ABmrGameState::PostInitializeComponents()
{
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	Super::PostInitializeComponents();
}

// Called when the game starts
void ABmrGameState::BeginPlay()
{
	Super::BeginPlay();

	BindOnGameStateTagChanged();

	StartGameStateTree();

	BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
}

// Overridable function called whenever this actor is being removed from a level
void ABmrGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RegisterGenericGameplayTagEvent().RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

// Called when the local player character is spawned, possessed, and replicated
void ABmrGameState::OnLocalPawnReady_Implementation(const FGameplayEventData& Payload)
{
	// On client, the tag might have replicated before pawn was ready
	if (!HasAuthority()
	    && HasMatchingGameplayTag(FBmrGameStateTag::ParentTag))
	{
		BroadcastGameStateChanged();
	}
}