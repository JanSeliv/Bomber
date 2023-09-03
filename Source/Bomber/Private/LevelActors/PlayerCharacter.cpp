// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/PlayerCharacter.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Controllers/MyAIController.h"
#include "Controllers/MyPlayerController.h"
#include "DataAssets/ItemDataAsset.h"
#include "DataAssets/PlayerDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/BombActor.h"
#include "LevelActors/ItemActor.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerCharacter)

// Default amount on picked up items
const FPowerUp FPowerUp::DefaultData = FPowerUp();

// Sets default values
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMySkeletalMeshComponent>(MeshComponentName)) // Init UMySkeletalMeshComponent instead of USkeletalMeshComponent
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Set the default AI controller class
	AIControllerClass = AMyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// Do not rotate player by camera
	bUseControllerRotationYaw = false;

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
		static const FVector MeshRelativeLocation(0, 0, -90.f);
		SkeletalMeshComponent->SetRelativeLocation_Direct(MeshRelativeLocation);
		static const FRotator MeshRelativeRotation(0, -90.f, 0);
		SkeletalMeshComponent->SetRelativeRotation_Direct(MeshRelativeRotation);

		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Initialize the nameplate mesh component
	NameplateMeshInternal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NameplateMeshComponent"));
	NameplateMeshInternal->SetupAttachment(RootComponent);
	static const FVector NameplateRelativeLocation(-60.f, 0.f, 150.f);
	NameplateMeshInternal->SetRelativeLocation_Direct(NameplateRelativeLocation);
	static const FVector NameplateRelativeScale(1.75f, 1.f, 1.f);
	NameplateMeshInternal->SetRelativeScale3D_Direct(NameplateRelativeScale);
	NameplateMeshInternal->SetUsingAbsoluteRotation(true);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Rotate player by movement
		MovementComponent->bOrientRotationToMovement = true;
		static const FRotator RotationRate(0.f, 540.f, 0.f);
		MovementComponent->RotationRate = RotationRate;

		// Do not push out clients from collision
		MovementComponent->MaxDepenetrationWithGeometryAsProxy = 0.f;
	}
}

// Initialize a player actor, could be called multiple times
void APlayerCharacter::ConstructPlayerCharacter()
{
	checkf(MapComponentInternal, TEXT("%s: 'MapComponentInternal' is null"), *FString(__FUNCTION__));
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionPlayerCharacter);
	MapComponentInternal->ConstructOwnerActor();
}

// Spawns bomb on character position
void APlayerCharacter::ServerSpawnBomb_Implementation()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		// Should not spawn bomb in PIE
		return;
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	const AController* OwnedController = GetController();
	if (!MapComponentInternal                     // The Map Component is not valid or transient
	    || PowerupsInternal.FireN <= 0            // Null length of explosion
	    || PowerupsInternal.BombN <= 0            // No more bombs
	    || !OwnedController                       // controller is not valid
	    || OwnedController->IsMoveInputIgnored()) // controller is blocked
	{
		return;
	}

	// Spawn bomb
	ABombActor* BombActor = AGeneratedMap::SpawnActorByType<ABombActor>(EAT::Bomb, MapComponentInternal->GetCell());
	if (!BombActor) // can return nullptr if the cell is not free
	{
		return;
	}

	UMapComponent* MapComponent = UMapComponent::GetMapComponent(BombActor);
	if (!MapComponent)
	{
		return;
	}

	// Updating explosion cells
	PowerupsInternal.BombN--;

	// Init Bomb
	BombActor->InitBomb(this);

	// Start listening this bomb
	if (!MapComponent->OnDeactivatedMapComponent.IsAlreadyBound(this, &ThisClass::OnBombDestroyed))
	{
		MapComponent->OnDeactivatedMapComponent.AddDynamic(this, &ThisClass::OnBombDestroyed);
	}
}

// Returns the Skeletal Mesh of bombers
UMySkeletalMeshComponent* APlayerCharacter::GetMySkeletalMeshComponent() const
{
	return Cast<UMySkeletalMeshComponent>(GetMesh());
}

// Actualize the player name for this character
void APlayerCharacter::UpdateNicknameOnNameplate()
{
	static const FName DefaultAIName(TEXT("AI"));
	FName NewNickname = DefaultAIName;

	if (const AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>())
	{
		NewNickname = MyPlayerState->GetPlayerFNameCustom();
	}

	SetNicknameOnNameplate(NewNickname);
}

// Returns level type associated with player, e.g: Water level type for Roger character
ELevelType APlayerCharacter::GetPlayerType() const
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	return PlayerRow ? PlayerRow->LevelType : ELT::None;
}

// Set and apply how a player has to look lik
void APlayerCharacter::ServerSetCustomPlayerMeshData_Implementation(const FCustomPlayerMeshData& CustomPlayerMeshData)
{
	PlayerMeshDataInternal = CustomPlayerMeshData;
	ApplyCustomPlayerMeshData();
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	// Call to super
	Super::BeginPlay();

	// Set the animation
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const TSubclassOf<UAnimInstance> AnimInstanceClass = UPlayerDataAsset::Get().GetAnimInstanceClass();
		MeshComp->SetAnimInstanceClass(AnimInstanceClass);
	}

	TryPossessController();

	if (HasAuthority())
	{
		OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerBeginOverlap);

		if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
		{
			MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);

			// Handle current game state if initialized with delay
			if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
			{
				OnGameStateChanged(ECurrentGameState::Menu);
			}
		}

		// Listen to handle possessing logic
		FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnPostLogin);
	}

	if (AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>())
	{
		MyPlayerState->OnPlayerNameChanged.AddDynamic(this, &ThisClass::SetNicknameOnNameplate);
	}
}

// Called when an instance of this class is placed (in editor) or spawned
void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructPlayerCharacter();
}

// Is called on a player character construction, could be called multiple times
void APlayerCharacter::OnConstructionPlayerCharacter()
{
	if (IS_TRANSIENT(this)        // This actor is transient
	    || !MapComponentInternal) // Is not valid for map construction
	{
		return;
	}

	// Set ID
	if (HasAuthority()
	    && CharacterIDInternal == INDEX_NONE)
	{
		const FCells PlayerCells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Player));
		CharacterIDInternal = PlayerCells.Num() - 1;
		ApplyCharacterID();
	}

	UpdateNicknameOnNameplate();

	// Spawn or destroy controller of specific ai with enabled visualization
#if WITH_EDITOR
	if (FEditorUtilsLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld] only
	    && CharacterIDInternal > 0)                // Is a bot
	{
		MyAIControllerInternal = Cast<AMyAIController>(GetController());
		if (!MapComponentInternal->bShouldShowRenders)
		{
			if (MyAIControllerInternal)
			{
				MyAIControllerInternal->Destroy();
			}
		}
		else if (!MyAIControllerInternal) // Is a bot with debug visualization and AI controller is not created yet
		{
			SpawnDefaultController();
			if (AController* PlayerController = GetController())
			{
				PlayerController->bIsEditorOnlyActor = true;
			}
		}
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]
}

// Called every frame, is disabled on start, tick interval is decreased
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MapComponentInternal)
	{
		// Update a player location on the Generated Map
		AGeneratedMap::Get().SetNearestCell(MapComponentInternal);

		MapComponentInternal->TryDisplayOwnedCell();
	}
}

// Returns properties that are replicated for the lifetime of the actor channel
void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PowerupsInternal);
	DOREPLIFETIME(ThisClass, CharacterIDInternal);
	DOREPLIFETIME(ThisClass, PlayerMeshDataInternal);
}

// Is overriden to handle the client login when is set new player state
void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ApplyCustomPlayerMeshData();
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void APlayerCharacter::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	ResetPowerups();

	if (UMySkeletalMeshComponent* MySkeletalMeshComponent = GetMySkeletalMeshComponent())
	{
		const ECollisionEnabled::Type NewType = bNewHidden ? ECollisionEnabled::NoCollision : ECollisionEnabled::PhysicsOnly;
		MySkeletalMeshComponent->SetCollisionEnabled(NewType);
	}

	if (!bNewHidden)
	{
		// Is added on Generated Map

		ConstructPlayerCharacter();

		TryPossessController();

		return;
	}

	// Is removed from Generated Map
	if (Controller)
	{
		Controller->UnPossess();
	}
}

// Called when this Pawn is possessed. Only called on the server (or in standalone)
void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ApplyCustomPlayerMeshData();

	UpdateNicknameOnNameplate();
}

// Triggers when this player character starts something overlap.
void APlayerCharacter::OnPlayerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	const AItemActor* OverlappedItem = Cast<AItemActor>(OtherActor);
	const EItemType ItemType = OverlappedItem ? OverlappedItem->GetItemType() : EItemType::None;
	if (ItemType == EItemType::None) // item is not valid
	{
		return;
	}

	const UItemDataAsset& ItemDataAsset = UItemDataAsset::Get();

	const int32 MaxAllowedItemsNum = ItemDataAsset.GetMaxAllowedItemsNum();
	auto IncrementIfAllowed = [MaxAllowedItemsNum](int32& NumRef)
	{
		if (NumRef < MaxAllowedItemsNum)
		{
			++NumRef;
		}
	};

	switch (ItemType)
	{
		case EItemType::Skate:
		{
			IncrementIfAllowed(PowerupsInternal.SkateN);
			break;
		}
		case EItemType::Bomb:
		{
			IncrementIfAllowed(PowerupsInternal.BombN);
			break;
		}
		case EItemType::Fire:
		{
			IncrementIfAllowed(PowerupsInternal.FireN);
			break;
		}
		default:
			break;
	}

	ApplyPowerups();
}

// Event triggered when the bomb has been explicitly destroyed.
void APlayerCharacter::OnBombDestroyed(UMapComponent* MapComponent, UObject* DestroyCauser/* = nullptr*/)
{
	if (!MapComponent
	    || MapComponent->GetActorType() != EAT::Bomb)
	{
		return;
	}

	// Stop listening this bomb
	if (MapComponent->OnDeactivatedMapComponent.IsAlreadyBound(this, &ThisClass::OnBombDestroyed))
	{
		MapComponent->OnDeactivatedMapComponent.RemoveDynamic(this, &ThisClass::OnBombDestroyed);
	}

	if (PowerupsInternal.BombN < UItemDataAsset::Get().GetMaxAllowedItemsNum())
	{
		++PowerupsInternal.BombN;
	}
}

// Listen to manage the tick
void APlayerCharacter::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu: // fallthrough
		case ECurrentGameState::GameStarting:
		{
			SetActorTickEnabled(false);
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetActorTickEnabled(true);
			break;
		}
		default:
			break;
	}
}

// Apply effect of picked up powerups
void APlayerCharacter::ApplyPowerups()
{
	// Apply speed
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		static constexpr float SpeedMultiplier = 100.F;
		const float SkateAdditiveStrength = UItemDataAsset::Get().GetSkateAdditiveStrength();
		const int32 SkateN = PowerupsInternal.SkateN * SpeedMultiplier + SkateAdditiveStrength;
		MovementComponent->MaxWalkSpeed = SkateN;
	}

	// Apply others
}

// Reset all picked up powerups
void APlayerCharacter::ResetPowerups()
{
	PowerupsInternal = FPowerUp::DefaultData;
	ApplyPowerups();
}

// Is called on clients to apply powerups
void APlayerCharacter::OnRep_Powerups()
{
	ApplyPowerups();
}

// Update player name on a 3D widget component
void APlayerCharacter::SetNicknameOnNameplate_Implementation(FName NewName)
{
	// BP implementation
	// ...
}

// Updates collision object type by current character ID
void APlayerCharacter::UpdateCollisionObjectType()
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (!ensureMsgf(CapsuleComp, TEXT("ASSERT: 'CapsuleComponent' is not valid"))
	    || CharacterIDInternal == INDEX_NONE)
	{
		return;
	}

	// Set the object collision type
	ECollisionChannel CollisionObjectType = CapsuleComp->GetCollisionObjectType();
	switch (CharacterIDInternal)
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

// Possess a player or AI controller in dependence of current Character ID
void APlayerCharacter::TryPossessController()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		// Should not spawn posses in PIE
		return;
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	if (!HasAuthority()
	    || CharacterIDInternal < 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AController* ControllerToPossess = nullptr;

	if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetMyPlayerController(CharacterIDInternal))
	{
		if (MyPC->bCinematicMode)
		{
			// Prevent crash on trying to posses the player during playing the sequencer
			return;
		}

		// Possess the player
		ControllerToPossess = MyPC;
	}
	else // Possess the AI
	{
		// Spawn AI if is needed
		if (!MyAIControllerInternal)
		{
			MyAIControllerInternal = World->SpawnActor<AMyAIController>(AIControllerClass, GetActorTransform());
		}

		ControllerToPossess = MyAIControllerInternal;
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

// Is called on game mode post login to handle character logic when new player is connected
void APlayerCharacter::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	TryPossessController();
}

// Set and apply new skeletal mesh from current data
void APlayerCharacter::ApplyCustomPlayerMeshData()
{
	UMySkeletalMeshComponent* MySkeletalMeshComp = Cast<UMySkeletalMeshComponent>(GetMesh());
	if (!ensureMsgf(MySkeletalMeshComp, TEXT("ASSERT: 'MySkeletalMeshComp' is not valid"))
	    || !MapComponentInternal)
	{
		return;
	}

	if (!PlayerMeshDataInternal.IsValid())
	{
		return;
	}

	MySkeletalMeshComp->InitMySkeletalMesh(PlayerMeshDataInternal);

	MapComponentInternal->SetLevelActorRow(PlayerMeshDataInternal.PlayerRow);
}

// Set and apply default skeletal mesh for this player
void APlayerCharacter::SetDefaultPlayerMeshData()
{
	const UPlayerDataAsset& PlayerDataAsset = UPlayerDataAsset::Get();
	const int32 MeshesNum = PlayerDataAsset.GetRowsNum();
	if (!MeshesNum)
	{
		return;
	}

	const bool bIsPlayer = IsLocallyControlled() || !CharacterIDInternal;
	const ELevelType PlayerFlag = UMyBlueprintFunctionLibrary::GetLevelType();
	constexpr ELevelType AIFlag = ELT::None;
	const ELevelType LevelType = bIsPlayer ? PlayerFlag : AIFlag;
	const UPlayerRow* Row = PlayerDataAsset.GetRowByLevelType<UPlayerRow>(TO_ENUM(ELevelType, LevelType));
	if (!Row)
	{
		return;
	}

	const int32 SkinsNum = Row->GetMaterialInstancesDynamicNum();
	FCustomPlayerMeshData CustomPlayerMeshData = FCustomPlayerMeshData::Empty;
	CustomPlayerMeshData.PlayerRow = Row;
	CustomPlayerMeshData.SkinIndex = CharacterIDInternal % SkinsNum;
	ServerSetCustomPlayerMeshData(CustomPlayerMeshData);
}

// Respond on changes in player mesh data to reset to set the mesh on client
void APlayerCharacter::OnRep_PlayerMeshData()
{
	ApplyCustomPlayerMeshData();
}

void APlayerCharacter::ApplyCharacterID()
{
	if (CharacterIDInternal == INDEX_NONE)
	{
		return;
	}

	SetDefaultPlayerMeshData();

	// Set a nameplate material
	if (ensureMsgf(NameplateMeshInternal, TEXT("ASSERT: 'NameplateMeshComponent' is not valid")))
	{
		const UPlayerDataAsset& PlayerDataAsset = UPlayerDataAsset::Get();
		const int32 NameplateMeshesNum = PlayerDataAsset.GetNameplateMaterialsNum();
		if (NameplateMeshesNum > 0)
		{
			const int32 MaterialNo = CharacterIDInternal < NameplateMeshesNum ? CharacterIDInternal : CharacterIDInternal % NameplateMeshesNum;
			if (UMaterialInterface* Material = PlayerDataAsset.GetNameplateMaterial(MaterialNo))
			{
				NameplateMeshInternal->SetMaterial(0, Material);
			}
		}
	}

	UpdateCollisionObjectType();
}

// Is called on clients to apply the characterID-dependent logic for this character
void APlayerCharacter::OnRep_CharacterID()
{
	ApplyCharacterID();
}

// Move the player character
void APlayerCharacter::MovePlayer(const FInputActionValue& ActionValue)
{
	// input is a Vector2D
	const FVector2D MovementVector = ActionValue.Get<FVector2D>();

	// Find out which way is forward
	const FRotator ForwardRotation = UCellsUtilsLibrary::GetLevelGridRotation();

	// Get forward vector
	const FVector ForwardDirection = FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::X);

	// Get right vector
	const FVector RightDirection = FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}
