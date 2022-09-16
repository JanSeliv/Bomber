// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/PlayerCharacter.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Controllers/MyAIController.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/DataAssetsContainer.h"
#include "UtilityLibraries/SingletonLibrary.h"
#include "LevelActors/BombActor.h"
#include "LevelActors/ItemActor.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Controllers/MyPlayerController.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2DArray.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#endif

// Default amount on picked up items
const FPowerUp FPowerUp::DefaultData = FPowerUp();

// Returns the dynamic material instance of a player with specified skin.
UMaterialInstanceDynamic* UPlayerRow::GetMaterialInstanceDynamic(int32 SkinIndex) const
{
	if (MaterialInstancesDynamicInternal.IsValidIndex(SkinIndex))
	{
		return MaterialInstancesDynamicInternal[SkinIndex];
	}

	return nullptr;
}

#if WITH_EDITOR
// Handle adding and changing material instance to prepare dynamic materials
void UPlayerRow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Continue only if [IsEditorNotPieWorld]
	if (!UEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	// If material instance was changed
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(ThisClass, MaterialInstanceInternal);
	const FProperty* Property = PropertyChangedEvent.Property;
	if (Property
	    && Property->IsA<FObjectProperty>()
	    && PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet
	    && Property->GetFName() == PropertyName)
	{
		// Force recreation of dynamic material instances
		MaterialInstancesDynamicInternal.Empty();
		TryCreateDynamicMaterials();
	}
}

// Create dynamic material instance for each ski if is not done before.
void UPlayerRow::TryCreateDynamicMaterials()
{
	const auto PlayerDataAsset = Cast<UPlayerDataAsset>(GetOuter());
	if (!PlayerDataAsset
	    || !MaterialInstanceInternal)
	{
		return;
	}

	static const FName SkinArrayParameterName = PlayerDataAsset->GetSkinArrayParameter();
	static const FName SkinIndexParameterName = PlayerDataAsset->GetSkinIndexParameter();
	static const bool bSkinParamNamesAreValid = !(SkinArrayParameterName.IsNone() && SkinIndexParameterName.IsNone());
	if (!ensureMsgf(bSkinParamNamesAreValid, TEXT("ASSERT: TryCreateDynamicMaterials: 'bSkinParamNamesAreValid' is false")))
	{
		return;
	}

	// Find the number of skins in the texture 2D array of player material instance.
	UTexture* FoundTexture = nullptr;
	MaterialInstanceInternal->GetTextureParameterValue(SkinArrayParameterName, FoundTexture);
	const auto Texture2DArray = Cast<UTexture2DArray>(FoundTexture);
	const int32 SkinTexturesNum = Texture2DArray ? Texture2DArray->GetArraySize() : 0;
	const int32 MaterialInstancesDynamicNum = MaterialInstancesDynamicInternal.Num();
	if (SkinTexturesNum == MaterialInstancesDynamicNum)
	{
		// The same amount, so all dynamic materials are already created
		return;
	}

	// Create dynamic materials
	const int32 InstancesToCreateNum = SkinTexturesNum - MaterialInstancesDynamicNum;
	for (int32 i = 0; i < InstancesToCreateNum; ++i)
	{
		UMaterialInstanceDynamic* MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialInstanceInternal, PlayerDataAsset);
		if (!ensureMsgf(MaterialInstanceDynamic, TEXT("ASSERT: Could not create 'MaterialInstanceDynamic'")))
		{
			continue;
		}

		MaterialInstanceDynamic->SetFlags(RF_Public | RF_Transactional);
		const int32 SkinPosition = MaterialInstancesDynamicInternal.Emplace(MaterialInstanceDynamic);
		MaterialInstanceDynamic->SetScalarParameterValue(SkinIndexParameterName, SkinPosition);
	}
}
#endif	//WITH_EDITOR

// Default constructor
UPlayerDataAsset::UPlayerDataAsset()
{
	ActorTypeInternal = EAT::Player;
	RowClassInternal = UPlayerRow::StaticClass();
}

// Returns the player data asset
const UPlayerDataAsset& UPlayerDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Player);
	const auto PlayerDataAsset = Cast<UPlayerDataAsset>(FoundDataAsset);
	checkf(PlayerDataAsset, TEXT("The Player Data Asset is not valid"));
	return *PlayerDataAsset;
}

// Get nameplate material by index, is used by nameplate meshes
UMaterialInterface* UPlayerDataAsset::GetNameplateMaterial(int32 Index) const
{
	if (NameplateMaterialsInternal.IsValidIndex(Index))
	{
		return NameplateMaterialsInternal[Index];
	}

	return nullptr;
}

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
		// Disable gravity
		MovementComponent->GravityScale = 0.f;

		// Rotate player by movement
		MovementComponent->bOrientRotationToMovement = true;
		static const FRotator RotationRate(0.f, 540.f, 0.f);
		MovementComponent->RotationRate = RotationRate;

		// Do not push out clients from collision
		MovementComponent->MaxDepenetrationWithGeometryAsProxy = 0.f;
	}
}

// Spawns bomb on character position
void APlayerCharacter::ServerSpawnBomb_Implementation()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (UEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		// Should not spawn bomb in PIE
		return;
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	const AController* OwnedController = GetController();
	if (!MapComponentInternal                       // The Map Component is not valid or transient
	    || PowerupsInternal.FireN <= 0              // Null length of explosion
	    || PowerupsInternal.BombN <= 0              // No more bombs
	    || !OwnedController                         // controller is not valid
	    || OwnedController->IsMoveInputIgnored())   // controller is blocked
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
	BombActor->InitBomb(PowerupsInternal.FireN, CharacterIDInternal);

	// Start listening this bomb
	if (!MapComponent->OnDeactivatedMapComponent.IsAlreadyBound(this, &ThisClass::OnBombDestroyed))
	{
		MapComponent->OnDeactivatedMapComponent.AddDynamic(this, &ThisClass::OnBombDestroyed);
	}
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

		if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
		{
			MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
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

	if (IS_TRANSIENT(this)        // This actor is transient
	    || !MapComponentInternal) // Is not valid for map construction
	{
		return;
	}

	// Construct the actor's map component
	const bool bIsConstructed = MapComponentInternal->OnConstruction();
	if (!bIsConstructed)
	{
		return;
	}

	// Set ID
	if (HasAuthority()
	    && CharacterIDInternal == INDEX_NONE)
	{
		const FCells PlayerCells = UCellsUtilsLibrary::GetAllCellsByActors(TO_FLAG(EAT::Player));
		CharacterIDInternal = PlayerCells.Num() - 1;
		ApplyCharacterID();
	}

	UpdateNicknameOnNameplate();

	// Spawn or destroy controller of specific ai with enabled visualization
#if WITH_EDITOR
	if (UEditorUtilsLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld] only
	    && CharacterIDInternal > 0)              // Is a bot
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

	// Update a player location on the level map
	AGeneratedMap::Get().SetNearestCell(MapComponentInternal);
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
		// Is added on level map
		TryPossessController();
		return;
	}

	// Is removed from level map
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
void APlayerCharacter::OnBombDestroyed(UMapComponent* MapComponent)
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
	if (UEditorUtilsLibrary::IsEditorNotPieWorld())
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

	if (AMyPlayerController* MyPC = USingletonLibrary::GetMyPlayerController(CharacterIDInternal))
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
		if (!IS_VALID(MyAIControllerInternal))
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
	const ELevelType PlayerFlag = USingletonLibrary::GetLevelType();
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

// Move the player character by the forward vector
void APlayerCharacter::MoveBackForward(const FInputActionValue& ActionValue)
{
	const float ScaleValue = ActionValue.GetMagnitude();

	// Find out which way is forward
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// Get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Direction, ScaleValue);
}

// Move the player character by the right vector.
void APlayerCharacter::MoveRightLeft(const FInputActionValue& ActionValue)
{
	const float ScaleValue = ActionValue.GetMagnitude();

	// Find out which way is right
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// Get right vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(Direction, ScaleValue);
}
