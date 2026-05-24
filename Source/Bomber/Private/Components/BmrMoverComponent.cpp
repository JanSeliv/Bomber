// Copyright (c) Yevhenii Selivanov

#include "Components/BmrMoverComponent.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrGeneratedMapDataAsset.h"
#include "DataAssets/BmrPlayerDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Structures/BmrMoverSyncState.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMoverComponent)

// Moves owner in given direction
void UBmrMoverComponent::RequestMoveByIntent(const FVector& Direction)
{
	CurrentMoveInput = Direction;
}

// Instantly teleports owner to given location, useful for respawning or level transitions
void UBmrMoverComponent::TeleportToLocation(const FVector& InLocation)
{
	const FVector HeightOffset = FVector::UpVector * UBmrGeneratedMapDataAsset::Get().GetActorsHeightOffset();
	const FVector TargetLocation = InLocation + HeightOffset;

	if (!BackendLiaisonComp)
	{
		// Might be null in editor preview or during initialization
		GetOwner()->SetActorLocation(TargetLocation);
		return;
	}

	// Block movement for teleport tick so previous input does not apply in next tick
	SetBlockMovement(true);

	const TSharedPtr<FTeleportEffect> TeleportEffect = MakeShared<FTeleportEffect>();
	TeleportEffect->TargetLocation = TargetLocation;
	TeleportEffect->bUseActorRotation = false;
	TeleportEffect->TargetRotation = FRotator::ZeroRotator;
	QueueInstantMovementEffect(TeleportEffect);
}

// When blocked, all movement inputs are ignored and the owner pawn will not move
void UBmrMoverComponent::SetBlockMovement(bool bShouldBlock)
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC
	    || bShouldBlock == IsBlockedMovement())
	{
		return;
	}

	if (bShouldBlock)
	{
		const TSubclassOf<UGameplayEffect> BlockMovementEffect = UBmrPlayerDataAsset::Get().GetBlockMovementEffect();
		ensureMsgf(BlockMovementEffect, TEXT("ASSERT: [%i] %hs:\n'BlockMovementEffect' is not valid!"), __LINE__, __FUNCTION__);
		ASC->ApplyGameplayEffectToSelf(BlockMovementEffect.GetDefaultObject(), /*Level*/ 1.f, ASC->MakeEffectContext());

		RequestMoveByIntent(FVector::ZeroVector);
	}
	else
	{
		ASC->RemoveActiveEffectsWithGrantedTags(BmrGameplayTags::GameplayEffect::Block::Movement.GetTag().GetSingleTagContainer());
	}
}

// Returns true if movement is currently disabled
bool UBmrMoverComponent::IsBlockedMovement() const
{
	const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	return !ASC || ASC->HasMatchingGameplayTag(BmrGameplayTags::GameplayEffect::Block::Movement);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the game starts
void UBmrMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerPlayer = CastChecked<APawn>(GetOwner());

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::Player_PawnReady,
	    this, [this, WeakTarget = TWeakObjectPtr(OwnerPlayer)](const FGameplayEventData& Payload)
	{
		if (Payload.Instigator == WeakTarget.Get())
		{
			OnPawnReady(Payload);
		}
	});

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(OwnerPlayer);
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPreRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPreRemovedFromLevel);

	OnPostMovement.AddUniqueDynamic(this, &ThisClass::OnPostMove);

	OnTeleportSucceeded.AddUniqueDynamic(this, &ThisClass::OnTeleported);

	FMover_ProcessGeneratedMovement ProcessGeneratedMoveDelegate;
	ProcessGeneratedMoveDelegate.BindDynamic(this, &ThisClass::OnProcessGeneratedMove);
	BindProcessGeneratedMovement(ProcessGeneratedMoveDelegate);

	OwnerPlayer->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::OnControllerChanged);
}

// Consumes cached data (inputs and states) to be processed by other systems such as movement modes
void UBmrMoverComponent::ProduceInput(const int32 DeltaTimeMS, FMoverInputCmdContext* Cmd)
{
	Super::ProduceInput(DeltaTimeMS, Cmd);

	// --- Reference: AMoverExamplesCharacter::OnProduceInput
	// Generate user commands. Called right before the Character movement simulation will tick (for a locally controlled pawn)
	// This code is happening outside of the Character movement simulation. All we are doing
	// is generating the input being fed into that simulation. That said, this means that A) the code below does not run on the server
	// (and non controlling clients) and B) the code is not rerun during reconcile/resimulates.

	const APawn* InOwnerPawn = GetOwner<APawn>();
	static const FCharacterDefaultInputs DoNothingInput;
	FCharacterDefaultInputs& CharacterInputs = Cmd->InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

	const AController* OwnedController = InOwnerPawn ? InOwnerPawn->GetController() : nullptr;
	if (!OwnedController)
	{
		if (GetOwnerRole() == ROLE_Authority
		    && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
		{
			// If we get here, that means this pawn is not currently possessed and we're choosing to provide default do-nothing input
			CharacterInputs = DoNothingInput;
		}

		// We don't have a local controller so we can't run the code below. This is ok. Simulated proxies will just use previous input when extrapolating
		return;
	}

	if (IsBlockedMovement())
	{
		// Pass a do nothing input
		CharacterInputs = DoNothingInput;
		return;
	}

	// Setup control rotation
	CharacterInputs.ControlRotation = FRotator::ZeroRotator;
	const APlayerController* PC = Cast<APlayerController>(OwnedController);
	if (PC)
	{
		CharacterInputs.ControlRotation = PC->GetControlRotation();
	}

	// Setup movement input
	const FRotator LevelGridRotation = UBmrCellUtilsLibrary::GetLevelGridRotation();
	const FVector FinalDirectionalIntent = LevelGridRotation.RotateVector(CurrentMoveInput);
	CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, FinalDirectionalIntent);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when this character is ready to be used
void UBmrMoverComponent::OnPawnReady_Implementation(const FGameplayEventData& Payload)
{
	const ABmrPawn* Pawn = Cast<ABmrPawn>(Payload.Instigator.Get());
	checkf(Pawn == GetOwner(), TEXT("ERROR: [%i] %hs:\n'Pawn' is not the same as Owner!"), __LINE__, __FUNCTION__);

	// Setup powerups
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'ASC' is null, make sure Owner implements ability system interface!"), __LINE__, __FUNCTION__);
	FOnGameplayAttributeValueChange& SkateAttributeDelegate = ASC->GetGameplayAttributeValueChangeDelegate(UBmrPowerupsAttributeSet::GetPowerup_SkateAttribute());
	if (!SkateAttributeDelegate.IsBoundToObject(this))
	{
		// Entered in menu for first time, so bind to the Skate attribute change
		SkateAttributeDelegate.AddUObject(this, &ThisClass::OnSkateAttributeChanged);
		CachedSkatePowerupAttribute = UBmrPowerupsAttributeSet::Get(ASC).GetPowerup_Skate();
	}
}

// Listen to react when entered to different game state
void UBmrMoverComponent::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const bool bShouldDisableMovement = !Payload.InstigatorTags.HasTag(FBmrGameStateTag::InGame);
	SetBlockMovement(bShouldDisableMovement);
}

// Called when owner is unregistered from the Generated Map, on both server and client
void UBmrMoverComponent::OnPreRemovedFromLevel_Implementation(UBmrMapComponent* MapComponent, UObject* DestroyCauser)
{
	SetBlockMovement(true);
}

// Event called after a pawn's controller has changed, on the server and owning client
void UBmrMoverComponent::OnControllerChanged_Implementation(APawn* Pawn, AController* OldController, AController* NewController)
{
	const bool bShouldDisableMovement = !NewController || !ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::InGame);
	SetBlockMovement(bShouldDisableMovement);
}

// Broadcast at the end of a simulation tick after movement has occurred, but allowing additions/modifications to the state
void UBmrMoverComponent::OnPostMove_Implementation(const FMoverTimeStep& TimeStep, FMoverSyncState& SyncState, FMoverAuxStateContext& AuxState)
{
	if (IsBlockedMovement())
	{
		return;
	}

	// Add powerup state to SyncState
	FBmrMoverSyncState& BmrSyncStateRef = SyncState.SyncStateCollection.FindOrAddMutableDataByType<FBmrMoverSyncState>();
	BmrSyncStateRef.SkatePowerupAttribute = CachedSkatePowerupAttribute;

	// Update player location on the Generated Map
	const APawn* InOwnerPawn = GetOwner<APawn>();
	UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(InOwnerPawn);
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	if (InOwnerPawn->HasAuthority())
	{
		// On server, update a player location on the Generated Map
		ABmrGeneratedMap::Get().SetNearestCell(UBmrMapComponent::GetMapComponent(InOwnerPawn));
	}
	else if (InOwnerPawn->IsLocallyControlled())
	{
		// On local client, directly set a player location for responsiveness while server replicates it
		const FBmrCell SnappedCell = UBmrCellUtilsLibrary::SnapActorOnLevel(InOwnerPawn);
		MapComponent->SetCell(SnappedCell);
	}
}

// Called when queued teleport effect successfully applies, on every endpoint that ran simulation
void UBmrMoverComponent::OnTeleported_Implementation(const FVector& FromLocation, const FQuat& FromRotation, const FVector& ToLocation, const FQuat& ToRotation)
{
	if (ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::InGame))
	{
		// Only release teleport-tick block once game state allows movement
		SetBlockMovement(false);
	}
}

// Called between movement mode's GenerateMove and SimulationTick on every endpoint, with chance to rewrite proposed move before it is consumed
void UBmrMoverComponent::OnProcessGeneratedMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove)
{
	if (IsBlockedMovement())
	{
		// Block is applied but velocity might be already accumulated, reset it
		OutProposedMove.LinearVelocity = FVector::ZeroVector;
		OutProposedMove.AngularVelocityDegrees = FVector::ZeroVector;
	}
}

// Is called by Move Input Action when player pressed the move input button, e.g: WASD or Arrow keys
void UBmrMoverComponent::OnMoveInputTriggered_Implementation(const FInputActionValue& ActionValue)
{
	if (IsBlockedMovement())
	{
		return;
	}

	RequestMoveByIntent(ActionValue.Get<FVector>());
}

// Is called by Move Input Action when player released the move input button, e.g: WASD or Arrow keys
void UBmrMoverComponent::OnMoveInputCompleted_Implementation(const FInputActionValue& ActionValue)
{
	RequestMoveByIntent(FVector::ZeroVector);
}

// Is called when the Skate attribute is changed, e.g: when player picked up a Skate powerup
void UBmrMoverComponent::OnSkateAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	CachedSkatePowerupAttribute = OnAttributeChangeData.NewValue;
}