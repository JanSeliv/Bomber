// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrPawn.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Components/BmrMapComponent.h"
#include "Components/BmrMoverComponent.h"
#include "Components/BmrPlayerNameWidgetComponent.h"
#include "Components/BmrSkeletalMeshComponent.h"
#include "Controllers/BmrAIController.h"
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrPlayerDataAsset.h"
#include "GameFramework/BmrGameMode.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPawn)

// Sets default values
ABmrPawn::ABmrPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	bAlwaysRelevant = true;

	// Set the default AI controller class
	AIControllerClass = ABmrAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// Do not rotate player by camera
	bUseControllerRotationYaw = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCylinder"));
	RootComponent = CapsuleComponent;

	MapComponent = CreateDefaultSubobject<UBmrMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	MeshComponent = CreateDefaultSubobject<UBmrSkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MapComponent->SetMeshComponent(MeshComponent);

	// Initialize 3D widget component for the player name
	PlayerName3DWidgetComponent = CreateDefaultSubobject<UBmrPlayerNameWidgetComponent>(TEXT("PlayerName3DWidgetComponent"));
	PlayerName3DWidgetComponent->SetupAttachment(RootComponent);

	// Initialize Mover Component: most setup is done in Details Panel as it is full of instanced properties
	MoverComponent = CreateDefaultSubobject<UBmrMoverComponent>(TEXT("MoverComponent"));
	SetReplicatingMovement(false); // Mover requires to disable to handle on its own
}

// Returns the Player Tag associated with player
const FBmrPlayerTag& ABmrPawn::GetPlayerTag() const
{
	const UBmrPlayerRow* PlayerRow = MapComponent ? MapComponent->GetMeshRow<UBmrPlayerRow>() : nullptr;
	return PlayerRow ? PlayerRow->PlayerTag : FBmrPlayerTag::None;
}

// Returns the Ability System Component from the Player State
UAbilitySystemComponent* ABmrPawn::GetAbilitySystemComponent() const
{
	const ABmrPlayerState* InPlayerState = GetPlayerState<ABmrPlayerState>();
	return InPlayerState ? InPlayerState->GetAbilitySystemComponent() : nullptr;
}

// Returns the Ability System Component from the Player State, crash if nullptr
UAbilitySystemComponent& ABmrPawn::GetAbilitySystemComponentChecked() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponent' is null!"), __LINE__, __FUNCTION__);
	return *ASC;
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the game starts or when spawned
void ABmrPawn::BeginPlay()
{
	// Call to super
	Super::BeginPlay();

	// Set the animation blueprint on very first character spawn
	if (UBmrSkeletalMeshComponent* MeshComp = GetMeshComponent())
	{
		const TSubclassOf<UAnimInstance> AnimInstanceClass = UBmrPlayerDataAsset::Get().GetAnimInstanceClass();
		MeshComp->SetAnimInstanceClass(AnimInstanceClass);
	}

	// Attempt to posses player or AI on very first spawn
	TryPossessController(EBmrPlayerType::Any);

	if (HasAuthority())
	{
		// Listen to handle possessing logic
		FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnPostLogin);
	}

	BIND_ON_PLAYER_STATE_READY_ID(this, ThisClass::OnPlayerStateReady, GetPlayerId());
}

// Called when an instance of this class is placed (in editor) or spawned
void ABmrPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this))
	{
		return;
	}

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	ABmrGeneratedMap::Get().AddToGrid(MapComponent);
}

// Is overriden to handle the client login when is set new player state
void ABmrPawn::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (ABmrPlayerState* MyPlayerState = Cast<ABmrPlayerState>(NewPlayerState))
	{
		MyPlayerState->OnPlayerStateInit();
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
void ABmrPawn::OnAddedToLevel_Implementation(UBmrMapComponent* InMapComponent)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'InMapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPreRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPreRemovedFromLevel);
	InMapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);
	InMapComponent->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnCellChanged);
	InMapComponent->OnActorTypeChanged.AddUniqueDynamic(this, &ThisClass::OnActorTypeChanged);

	GetMeshComponentChecked().SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	if (!MapComponent->GetReplicatedMeshData().IsValid())
	{
		SetDefaultPlayerMeshData();
	}

	if (ABmrPlayerState* MyPlayerState = GetPlayerState<ABmrPlayerState>())
	{
		checkf(PlayerName3DWidgetComponent, TEXT("ERROR: [%i] %hs:\n'PlayerName3DWidgetComponent' is null!"), __LINE__, __FUNCTION__);
		PlayerName3DWidgetComponent->Init(MyPlayerState);
	}

	UpdateCollisionObjectType();

	// Spawn or destroy controller of specific ai with enabled visualization
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld] only
	    && UBmrActorUtilsLibrary::GetIndexByLevelActor(InMapComponent) > 0) // Is a bot
	{
		AIController = Cast<AAIController>(GetController());
		if (!InMapComponent->bShouldShowRenders)
		{
			if (AIController)
			{
				AIController->Destroy();
			}
		}
		else if (!AIController) // Is a bot with debug visualization and AI controller is not created yet
		{
			SpawnDefaultController();
			if (AController* PlayerController = GetController())
			{
				PlayerController->bIsEditorOnlyActor = true;
			}
		}
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	TryPossessController(EBmrPlayerType::Any);

	ApplyPreset();

	UBmrGlobalEventsSubsystem::Get().ReadyHandler.Broadcast_OnPawnAdded(*this);
}

// Is called when the Row from current Data Asset is changed for owner on the level, on both server and clients
void ABmrPawn::OnActorTypeChanged_Implementation(UBmrMapComponent* InMapComponent, const UBmrLevelActorRow* NewRow, const class UBmrLevelActorRow* PreviousRow)
{
	// Handle character change: apply new config to update attributes
	ApplyPreset();
}

// Is called on server when ANY human player joined the session
void ABmrPawn::OnPostLogin_Implementation(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	TryPossessController(EBmrPlayerType::Human);

	// If successfully replaced the bot by human, update the mesh
	if (GetController() == NewPlayer)
	{
		SetDefaultPlayerMeshData();
	}
}

// Is called on server when human player, previously possessed by this character, left the session
void ABmrPawn::OnPostLogout_Implementation(APlayerController* ExitingPlayer)
{
	// Player is leaving, possess the bot and update the mesh
	TryPossessController(EBmrPlayerType::Bot);
	SetDefaultPlayerMeshData();
}

// Is called when the player was destroyed
void ABmrPawn::OnPreRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	if (ABmrGameState::GetCurrentGameState() != EBmrCurrentGameState::InGame)
	{
		// Ignore, is not gameplay destroy, likely level is regenerated
		return;
	}

	// Mark this player as dead in own PlayerState
	if (ABmrPlayerState* InPlayerState = GetPlayerState<ABmrPlayerState>())
	{
		InPlayerState->SetPlayerDead(true);
	}

	// In the KillerPlayerState, mark this player as killed by DestroyCauser
	if (ABmrPlayerState* CauserPlayerState = Cast<ABmrPlayerState>(DestroyCauser))
	{
		CauserPlayerState->SetOpponentKilled(this);
	}
}

// Is used for cleaning up the character's data after it was removed from the level
void ABmrPawn::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	// -- Handle cleanup after removed player from the level

	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'InMapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.RemoveAll(this);
	InMapComponent->OnCellChanged.RemoveAll(this);
	InMapComponent->OnActorTypeChanged.RemoveAll(this);

	OnActorBeginOverlap.RemoveAll(this);

	UBmrGlobalEventsSubsystem::Get().BP_OnGameStateChanged.RemoveAll(this);

	GetMeshComponentChecked().SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->CancelAllAbilities();
	}
}

// Is called for everytime when character changed its cell on the Generated Map
void ABmrPawn::OnCellChanged_Implementation(UBmrMapComponent* InMapComponent, const FBmrCell& NewCell, const FBmrCell& PreviousCell)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'InMapComponent' is guaranteed to be valid!"), __LINE__, __FUNCTION__);
	if (HasActorBegunPlay())
	{
		// Visualize the cell changes during the gameplay
		MapComponent->TryDisplayOwnedCell(/*bClearPrevious*/ true);
	}
}

// Is called when the player state is fully initialized
void ABmrPawn::OnPlayerStateReady_Implementation(ABmrPlayerState* InPlayerState, int32 PlayerId)
{
	checkf(InPlayerState == GetPlayerState(), TEXT("ERROR: [%i] %hs:\n'InPlayerState' is different than owned!"), __LINE__, __FUNCTION__);

	checkf(PlayerName3DWidgetComponent, TEXT("ERROR: [%i] %hs:\n'PlayerName3DWidgetComponent' is null!"), __LINE__, __FUNCTION__);
	PlayerName3DWidgetComponent->Init(InPlayerState);

	ApplyPreset();
}

/*********************************************************************************************
 * Protected functions
 ********************************************************************************************* */

// Updates collision object type by current character ID
void ABmrPawn::UpdateCollisionObjectType()
{
	const int32 PlayerId = GetPlayerId();
	if (PlayerId < 0) // Might be replicating yet
	{
		return;
	}

	// Set the object collision type
	UCapsuleComponent* CapsuleComp = CastChecked<UCapsuleComponent>(RootComponent);
	ECollisionChannel CollisionObjectType = CapsuleComp->GetCollisionObjectType();
	switch (PlayerId)
	{
		case 0:
			CollisionObjectType = ECC_Player0;
			break;
		case 1:
			CollisionObjectType = ECC_Player1;
			break;
		case 2:
			CollisionObjectType = ECC_Player2;
			break;
		case 3:
			CollisionObjectType = ECC_Player3;
			break;
		default:
			break;
	}

	CapsuleComp->SetCollisionObjectType(CollisionObjectType);
}

// Sets current config: each character has its own configuration, like different starting attributes
void ABmrPawn::ApplyPreset()
{
	if (!HasAuthority())
	{
		return;
	}

	// Firstly, reset attributes
	if (ABmrPlayerState* MyPlayerState = GetPlayerState<ABmrPlayerState>())
	{
		MyPlayerState->ApplyDefaultAttributes();
	}

	// Secondly, apply config gameplay effect
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const UBmrPlayerRow* PlayerRow = UBmrPlayerDataAsset::Get().GetRowByPlayerTag(GetPlayerTag());
	const TSubclassOf<UGameplayEffect> ConfigGameplayEffect = PlayerRow ? PlayerRow->ConfigGameplayEffect : nullptr;
	if (ASC && ConfigGameplayEffect)
	{
		ASC->ApplyGameplayEffectToSelf(ConfigGameplayEffect.GetDefaultObject(), /*Level*/ 1, ASC->MakeEffectContext());
	}
}

/*********************************************************************************************
 * Controller (AI/Player)
 ********************************************************************************************* */

// Is overridden to determine additional conditions for the player-controlled character
bool ABmrPawn::IsPlayerControlled() const
{
	if (Super::IsPlayerControlled())
	{
		return true;
	}

	// Player state is not initialized yet (so Super returned false), but 0 is always a player
	return !GetPlayerState() && GetPlayerId() == 0;
}

// Possess a player or AI controller in dependence of current Character ID
void ABmrPawn::TryPossessController(EBmrPlayerType PlayerType)
{
	if (!HasAuthority()
	    || !IsActorInitialized() // Engine doesn't allow possess before BeginPlay\PostInitializeComponents
	    || UUtilsLibrary::IsEditorNotPieWorld()
	    || !ensureMsgf(PlayerType != EBmrPlayerType::None, TEXT("ASSERT: [%i] %hs:\n'PlayerType' is None, can't possess!"), __LINE__, __FUNCTION__))
	{
		// Should not possess in PIE
		return;
	}

	const int32 PlayerId = GetPlayerId();
	const ABmrGameMode* MyGameMode = UBmrBlueprintFunctionLibrary::GetGameMode();
	if (!ensureMsgf(PlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'PlayerId' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(MyGameMode, TEXT("ASSERT: [%i] %hs:\n'MyGameMode' is not valid! Make sure '%s' class is assigned to the '%s' level"), __LINE__, __FUNCTION__, *ABmrGameMode::StaticClass()->GetName(), *GetWorld()->GetMapName()))
	{
		return;
	}

	if (PlayerType == EBmrPlayerType::Any)
	{
		PlayerType = IsPlayerControlled() ? EBmrPlayerType::Human : EBmrPlayerType::Bot;
	}

	AController* ControllerToPossess = nullptr;
	switch (PlayerType)
	{
		default: checkNoEntry(); // Fallthrough

		case EBmrPlayerType::Human:
		{
			ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetPlayerController(PlayerId);
			if (MyPC
			    && !MyPC->bCinematicMode) // Don't possess player if it's the Render Movie
			{
				ControllerToPossess = MyPC;
			}
			break;
		}

		case EBmrPlayerType::Bot:
		{
			if (!AIController // Is not spawned yet
			    || !AIController->IsA(AIControllerClass)) // Spawned, but wrong AI controller assigned
			{
				// Spawn AI controller
				AIController = GetWorld()->SpawnActor<AAIController>(AIControllerClass, GetActorTransform());
			}

			ControllerToPossess = AIController;
		}
		break;
	}

	if (!ControllerToPossess
	    || ControllerToPossess == Controller)
	{
		return;
	}

	if (Controller)
	{
		// At first, unpossess previous controller
		Controller->UnPossess();
	}

	ControllerToPossess->Possess(this);
}

/*********************************************************************************************
 * Movement
 ********************************************************************************************* */

// Is overridden to return the velocity from Mover Component instead
FVector ABmrPawn::GetVelocity() const
{
	if (MoverComponent)
	{
		return MoverComponent->GetVelocity();
	}

	return Super::GetVelocity();
}

/*********************************************************************************************
 * Player ID
 ********************************************************************************************* */

// Returns own character ID, e.g: 0, 1, 2, 3
int32 ABmrPawn::GetPlayerId() const
{
	if (const ABmrPlayerState* MyPlayerState = GetPlayerState<ABmrPlayerState>())
	{
		return MyPlayerState->GetPlayerId();
	}

	// Player state is not initialized yet, return it directly from the order on the level
	return UBmrActorUtilsLibrary::GetIndexByLevelActor(MapComponent);
}

/*********************************************************************************************
 * Player Mesh
 ********************************************************************************************* */

// Returns the Skeletal Mesh Component, or crash if can not be accessed
UBmrSkeletalMeshComponent& ABmrPawn::GetMeshComponentChecked() const
{
	UBmrSkeletalMeshComponent* SkeletalMeshComponent = GetMeshComponent();
	checkf(SkeletalMeshComponent, TEXT("ERROR: [%i] %hs:\n'SkeletalMeshComponent' is null!"), __LINE__, __FUNCTION__);
	return *SkeletalMeshComponent;
}

// Set and apply default skeletal mesh for this player
void ABmrPawn::SetDefaultPlayerMeshData(bool bForcePlayerSkin /* = false*/)
{
	if (!HasAuthority())
	{
		return;
	}

	const UBmrPlayerDataAsset& PlayerDataAsset = UBmrPlayerDataAsset::Get();
	const int32 MeshesNum = PlayerDataAsset.GetRowsNum();
	const int32 PlayerId = GetPlayerId();
	if (!ensureMsgf(MeshesNum > 0, TEXT("ASSERT: [%i] %hs:\n'MeshesNum' is empty!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(PlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'PlayerId' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const bool bIsPlayer = IsPlayerControlled() || PlayerId == 0;
	const EBmrLevelType PlayerFlag = UBmrBlueprintFunctionLibrary::GetLevelType();
	constexpr EBmrLevelType AIFlag = ELT::None;
	EBmrLevelType LevelType = bIsPlayer ? PlayerFlag : AIFlag;

	if (bForcePlayerSkin)
	{
		// Force each bot to look like different player
		LevelType = static_cast<EBmrLevelType>(1 << PlayerId);
	}

	const UBmrPlayerRow* Row = PlayerDataAsset.GetRowByLevelType<UBmrPlayerRow>(TO_ENUM(EBmrLevelType, LevelType));
	if (!ensureMsgf(Row, TEXT("ASSERT: [%i] %hs:\n'Row' is not found!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const int32 SkinsNum = Row->GetSkinTexturesNum();
	FBmrMeshData MeshData = FBmrMeshData::Empty;
	MeshData.Row = Row;
	MeshData.SkinIndex = PlayerId % SkinsNum;

	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->SetReplicatedMeshData(MeshData);
}

/*********************************************************************************************
 * Bomb Placement
 ********************************************************************************************* */

// Spawns bomb on character position
void ABmrPawn::SpawnBomb()
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);

	// Activate bomb ability
	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.EventMagnitude = UBmrCellUtilsLibrary::GetIndexByCellOnLevel(MapComponent->GetCell());
	GetAbilitySystemComponentChecked().HandleGameplayEvent(BmrGameplayTags::Event::Bomb_Placed, &EventData);
}