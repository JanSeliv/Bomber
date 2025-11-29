// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/PlayerCharacter.h"

// Bomber
#include "Components/BmrMoverComponent.h"
#include "Components/BmrPlayerNameWidgetComponent.h"
#include "Components/MapComponent.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Controllers/MyAIController.h"
#include "Controllers/MyPlayerController.h"
#include "DataAssets/PlayerDataAsset.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "GeneratedMap.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/LevelActorsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerCharacter)

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	bAlwaysRelevant = true;

	// Set the default AI controller class
	AIControllerClass = AMyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// Do not rotate player by camera
	bUseControllerRotationYaw = false;

	CapsuleComponentInternal = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCylinder"));
	RootComponent = CapsuleComponentInternal;

	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	MeshComponentInternal = CreateDefaultSubobject<UMySkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponentInternal->SetupAttachment(RootComponent);
	MapComponentInternal->SetMeshComponent(MeshComponentInternal);

	// Initialize 3D widget component for the player name
	PlayerName3DWidgetComponentInternal = CreateDefaultSubobject<UBmrPlayerNameWidgetComponent>(TEXT("PlayerName3DWidgetComponent"));
	PlayerName3DWidgetComponentInternal->SetupAttachment(RootComponent);

	// Initialize Mover Component: most setup is done in Details Panel as it is full of instanced properties
	MoverComponentInternal = CreateDefaultSubobject<UBmrMoverComponent>(TEXT("MoverComponent"));
	SetReplicatingMovement(false); // Mover requires to disable to handle on its own
}

// Returns the Player Tag associated with player
const FPlayerTag& APlayerCharacter::GetPlayerTag() const
{
	const UPlayerRow* PlayerRow = MapComponentInternal ? MapComponentInternal->GetMeshRow<UPlayerRow>() : nullptr;
	return PlayerRow ? PlayerRow->PlayerTag : FPlayerTag::None;
}

// Returns the Ability System Component from the Player State
UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	const AMyPlayerState* InPlayerState = GetPlayerState<AMyPlayerState>();
	return InPlayerState ? InPlayerState->GetAbilitySystemComponent() : nullptr;
}

// Returns the Ability System Component from the Player State, crash if nullptr
UAbilitySystemComponent& APlayerCharacter::GetAbilitySystemComponentChecked() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponent' is null!"), __LINE__, __FUNCTION__);
	return *ASC;
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	// Call to super
	Super::BeginPlay();

	// Set the animation blueprint on very first character spawn
	if (UMySkeletalMeshComponent* MeshComp = GetMeshComponent())
	{
		const TSubclassOf<UAnimInstance> AnimInstanceClass = UPlayerDataAsset::Get().GetAnimInstanceClass();
		MeshComp->SetAnimInstanceClass(AnimInstanceClass);
	}

	// Attempt to posses player or AI on very first spawn
	TryPossessController(EPlayerType::Any);

	if (HasAuthority())
	{
		// Listen to handle possessing logic
		FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnPostLogin);
	}

	BIND_ON_PLAYER_STATE_READY_ID(this, ThisClass::OnPlayerStateReady, GetPlayerId());
}

// Called when an instance of this class is placed (in editor) or spawned
void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this))
	{
		return;
	}

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	AGeneratedMap::Get().AddToGrid(MapComponentInternal);
}

// Is overriden to handle the client login when is set new player state
void APlayerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (AMyPlayerState* MyPlayerState = Cast<AMyPlayerState>(NewPlayerState))
	{
		MyPlayerState->OnPlayerStateInit();
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
void APlayerCharacter::OnAddedToLevel_Implementation(UMapComponent* MapComponent)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPreRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPreRemovedFromLevel);
	MapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);
	MapComponent->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnCellChanged);
	MapComponent->OnActorTypeChanged.AddUniqueDynamic(this, &ThisClass::OnActorTypeChanged);

	GetMeshComponentChecked().SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	if (!MapComponentInternal->GetReplicatedMeshData().IsValid())
	{
		SetDefaultPlayerMeshData();
	}

	if (AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>())
	{
		checkf(PlayerName3DWidgetComponentInternal, TEXT("ERROR: [%i] %hs:\n'PlayerName3DWidgetComponentInternal' is null!"), __LINE__, __FUNCTION__);
		PlayerName3DWidgetComponentInternal->Init(MyPlayerState);
	}

	UpdateCollisionObjectType();

	// Spawn or destroy controller of specific ai with enabled visualization
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld] only
	    && ULevelActorsUtilsLibrary::GetIndexByLevelActor(MapComponent) > 0) // Is a bot
	{
		AIControllerInternal = Cast<AAIController>(GetController());
		if (!MapComponent->bShouldShowRenders)
		{
			if (AIControllerInternal)
			{
				AIControllerInternal->Destroy();
			}
		}
		else if (!AIControllerInternal) // Is a bot with debug visualization and AI controller is not created yet
		{
			SpawnDefaultController();
			if (AController* PlayerController = GetController())
			{
				PlayerController->bIsEditorOnlyActor = true;
			}
		}
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	TryPossessController(EPlayerType::Any);

	ApplyCharacterConfig();

	UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.Broadcast_OnCharacterAdded(*this);
}

// Is called when the Row from current Data Asset is changed for owner on the level, on both server and clients
void APlayerCharacter::OnActorTypeChanged_Implementation(UMapComponent* MapComponent, const ULevelActorRow* NewRow, const class ULevelActorRow* PreviousRow)
{
	// Handle character change: apply new config to update attributes
	ApplyCharacterConfig();
}

// Is called on server when ANY human player joined the session
void APlayerCharacter::OnPostLogin_Implementation(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	TryPossessController(EPlayerType::Human);

	// If successfully replaced the bot by human, update the mesh
	if (GetController() == NewPlayer)
	{
		SetDefaultPlayerMeshData();
	}
}

// Is called on server when human player, previously possessed by this character, left the session
void APlayerCharacter::OnPostLogout_Implementation(APlayerController* ExitingPlayer)
{
	// Player is leaving, possess the bot and update the mesh
	TryPossessController(EPlayerType::Bot);
	SetDefaultPlayerMeshData();
}

// Is called when the player was destroyed
void APlayerCharacter::OnPreRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	if (AMyGameStateBase::GetCurrentGameState() != ECurrentGameState::InGame)
	{
		// Ignore, is not gameplay destroy, likely level is regenerated
		return;
	}

	// Mark this player as dead in own PlayerState
	if (AMyPlayerState* InPlayerState = GetPlayerState<AMyPlayerState>())
	{
		InPlayerState->SetCharacterDead(true);
	}

	// In the KillerPlayerState, mark this player as killed by DestroyCauser
	if (AMyPlayerState* CauserPlayerState = Cast<AMyPlayerState>(DestroyCauser))
	{
		CauserPlayerState->SetOpponentKilled(this);
	}
}

// Is used for cleaning up the character's data after it was removed from the level
void APlayerCharacter::OnPostRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	// -- Handle cleanup after removed player from the level

	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.RemoveAll(this);
	MapComponent->OnCellChanged.RemoveAll(this);
	MapComponent->OnActorTypeChanged.RemoveAll(this);

	OnActorBeginOverlap.RemoveAll(this);

	UGlobalEventsSubsystem::Get().BP_OnGameStateChanged.RemoveAll(this);

	GetMeshComponentChecked().SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->CancelAllAbilities();
	}
}

// Is called for everytime when character changed its cell on the Generated Map
void APlayerCharacter::OnCellChanged_Implementation(UMapComponent* MapComponent, const FCell& NewCell, const FCell& PreviousCell)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is guaranteed to be valid!"), __LINE__, __FUNCTION__);
	if (HasActorBegunPlay())
	{
		// Visualize the cell changes during the gameplay
		MapComponentInternal->TryDisplayOwnedCell(/*bClearPrevious*/ true);
	}
}

// Is called when the player state is fully initialized
void APlayerCharacter::OnPlayerStateReady_Implementation(AMyPlayerState* InPlayerState, int32 CharacterID)
{
	checkf(InPlayerState == GetPlayerState(), TEXT("ERROR: [%i] %hs:\n'InPlayerState' is different than owned!"), __LINE__, __FUNCTION__);

	checkf(PlayerName3DWidgetComponentInternal, TEXT("ERROR: [%i] %hs:\n'PlayerName3DWidgetComponentInternal' is null!"), __LINE__, __FUNCTION__);
	PlayerName3DWidgetComponentInternal->Init(InPlayerState);

	ApplyCharacterConfig();
}

/*********************************************************************************************
 * Protected functions
 ********************************************************************************************* */

// Updates collision object type by current character ID
void APlayerCharacter::UpdateCollisionObjectType()
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
void APlayerCharacter::ApplyCharacterConfig()
{
	if (!HasAuthority())
	{
		return;
	}

	// Firstly, reset attributes
	if (AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>())
	{
		MyPlayerState->ApplyDefaultAttributes();
	}

	// Secondly, apply config gameplay effect
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const UPlayerRow* PlayerRow = UPlayerDataAsset::Get().GetRowByPlayerTag(GetPlayerTag());
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
bool APlayerCharacter::IsPlayerControlled() const
{
	if (Super::IsPlayerControlled())
	{
		return true;
	}

	// Player state is not initialized yet (so Super returned false), but 0 is always a player
	return !GetPlayerState() && GetPlayerId() == 0;
}

// Possess a player or AI controller in dependence of current Character ID
void APlayerCharacter::TryPossessController(EPlayerType PlayerType)
{
	if (!HasAuthority()
	    || !IsActorInitialized() // Engine doesn't allow possess before BeginPlay\PostInitializeComponents
	    || UUtilsLibrary::IsEditorNotPieWorld()
	    || !ensureMsgf(PlayerType != EPlayerType::None, TEXT("ASSERT: [%i] %hs:\n'PlayerType' is None, can't possess!"), __LINE__, __FUNCTION__))
	{
		// Should not possess in PIE
		return;
	}

	const int32 PlayerId = GetPlayerId();
	const AMyGameModeBase* MyGameMode = UMyBlueprintFunctionLibrary::GetMyGameMode();
	if (!ensureMsgf(PlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'PlayerId' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(MyGameMode, TEXT("ASSERT: [%i] %hs:\n'MyGameMode' is not valid! Make sure '%s' class is assigned to the '%s' level"), __LINE__, __FUNCTION__, *AMyGameModeBase::StaticClass()->GetName(), *GetWorld()->GetMapName()))
	{
		return;
	}

	if (PlayerType == EPlayerType::Any)
	{
		PlayerType = IsPlayerControlled() ? EPlayerType::Human : EPlayerType::Bot;
	}

	AController* ControllerToPossess = nullptr;
	switch (PlayerType)
	{
		default: checkNoEntry(); // Fallthrough

		case EPlayerType::Human:
		{
			AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetMyPlayerController(PlayerId);
			if (MyPC
			    && !MyPC->bCinematicMode) // Don't possess player if it's the Render Movie
			{
				ControllerToPossess = MyPC;
			}
			break;
		}

		case EPlayerType::Bot:
		{
			if (!AIControllerInternal // Is not spawned yet
			    || !AIControllerInternal->IsA(AIControllerClass)) // Spawned, but wrong AI controller assigned
			{
				// Spawn AI controller
				AIControllerInternal = GetWorld()->SpawnActor<AAIController>(AIControllerClass, GetActorTransform());
			}

			ControllerToPossess = AIControllerInternal;
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
FVector APlayerCharacter::GetVelocity() const
{
	if (MoverComponentInternal)
	{
		return MoverComponentInternal->GetVelocity();
	}

	return Super::GetVelocity();
}

/*********************************************************************************************
 * Player ID
 ********************************************************************************************* */

// Returns own character ID, e.g: 0, 1, 2, 3
int32 APlayerCharacter::GetPlayerId() const
{
	if (const AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>())
	{
		return MyPlayerState->GetPlayerId();
	}

	// Player state is not initialized yet, return it directly from the order on the level
	return ULevelActorsUtilsLibrary::GetIndexByLevelActor(MapComponentInternal);
}

/*********************************************************************************************
 * Player Mesh
 ********************************************************************************************* */

// Returns the Skeletal Mesh Component, or crash if can not be accessed
UMySkeletalMeshComponent& APlayerCharacter::GetMeshComponentChecked() const
{
	UMySkeletalMeshComponent* SkeletalMeshComponent = GetMeshComponent();
	checkf(SkeletalMeshComponent, TEXT("ERROR: [%i] %hs:\n'SkeletalMeshComponent' is null!"), __LINE__, __FUNCTION__);
	return *SkeletalMeshComponent;
}

// Set and apply default skeletal mesh for this player
void APlayerCharacter::SetDefaultPlayerMeshData(bool bForcePlayerSkin /* = false*/)
{
	if (!HasAuthority())
	{
		return;
	}

	const UPlayerDataAsset& PlayerDataAsset = UPlayerDataAsset::Get();
	const int32 MeshesNum = PlayerDataAsset.GetRowsNum();
	const int32 PlayerId = GetPlayerId();
	if (!ensureMsgf(MeshesNum > 0, TEXT("ASSERT: [%i] %hs:\n'MeshesNum' is empty!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(PlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'PlayerId' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const bool bIsPlayer = IsPlayerControlled() || PlayerId == 0;
	const ELevelType PlayerFlag = UMyBlueprintFunctionLibrary::GetLevelType();
	constexpr ELevelType AIFlag = ELT::None;
	ELevelType LevelType = bIsPlayer ? PlayerFlag : AIFlag;

	if (bForcePlayerSkin)
	{
		// Force each bot to look like different player
		LevelType = static_cast<ELevelType>(1 << PlayerId);
	}

	const UPlayerRow* Row = PlayerDataAsset.GetRowByLevelType<UPlayerRow>(TO_ENUM(ELevelType, LevelType));
	if (!ensureMsgf(Row, TEXT("ASSERT: [%i] %hs:\n'Row' is not found!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const int32 SkinsNum = Row->GetSkinTexturesNum();
	FBmrMeshData MeshData = FBmrMeshData::Empty;
	MeshData.Row = Row;
	MeshData.SkinIndex = PlayerId % SkinsNum;

	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->SetReplicatedMeshData(MeshData);
}

/*********************************************************************************************
 * Bomb Placement
 ********************************************************************************************* */

// Spawns bomb on character position
void APlayerCharacter::SpawnBomb()
{
	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);

	// Activate bomb ability
	FGameplayEventData EventData;
	EventData.EventMagnitude = UCellsUtilsLibrary::GetIndexByCellLevel(MapComponentInternal->GetCell());
	GetAbilitySystemComponentChecked().HandleGameplayEvent(BmrGameplayTags::Event::Bomb_Placed, &EventData);
}