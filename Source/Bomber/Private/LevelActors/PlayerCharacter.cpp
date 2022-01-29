// Copyright 2021 Yevhenii Selivanov.

#include "LevelActors/PlayerCharacter.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Controllers/MyAIController.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/BombActor.h"
#include "LevelActors/ItemActor.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Controllers/MyPlayerController.h"
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
	if (!USingletonLibrary::IsEditorNotPieWorld())
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
	const ULevelActorDataAsset* FoundDataAsset = USingletonLibrary::GetDataAssetByActorType(EActorType::Player);
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
	}
}

// Spawns bomb on character position
void APlayerCharacter::ServerSpawnBomb_Implementation()
{
	const AController* OwnedController = GetController();
	if (!MapComponentInternal                       // The Map Component is not valid or transient
	    || PowerupsInternal.FireN <= 0              // Null length of explosion
	    || PowerupsInternal.BombN <= 0              // No more bombs
	    || USingletonLibrary::IsEditorNotPieWorld() // Should not spawn bomb in PIE
	    || !OwnedController                         // controller is not valid
	    || OwnedController->IsMoveInputIgnored())   // controller is blocked
	{
		return;
	}

	// Spawn bomb
	if (ABombActor* BombActor = AGeneratedMap::SpawnActorByType<ABombActor>(EAT::Bomb, MapComponentInternal->Cell)) // can return nullptr if the cell is not free
	{
		// Updating explosion cells
		PowerupsInternal.BombN--;

		// Init Bomb
		BombActor->MulticastInitBomb(PowerupsInternal.FireN, CharacterIDInternal);
		BombActor->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnBombDestroyed);
	}
}

// Set and apply new skeletal mesh by specified data
void APlayerCharacter::InitMySkeletalMesh(const FCustomPlayerMeshData& CustomPlayerMeshData)
{
	const auto MySkeletalMeshComp = Cast<UMySkeletalMeshComponent>(GetMesh());
	if (!ensureMsgf(MySkeletalMeshComp, TEXT("ASSERT: 'MySkeletalMeshComp' is not valid")))
	{
		return;
	}

	MySkeletalMeshComp->InitMySkeletalMesh(CustomPlayerMeshData);
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	// Call to super
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Set the animation
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const TSubclassOf<UAnimInstance> AnimInstanceClass = UPlayerDataAsset::Get().GetAnimInstanceClass();
		MeshComp->SetAnimInstanceClass(AnimInstanceClass);
	}

	TryPossessController();

	OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerBeginOverlap);

	if (HasAuthority())
	{
		if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
		{
			MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
		}

		// Listen to handle possessing logic
		FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnPostLogin);
	}

	if (AMyPlayerState* MyPlayerState = USingletonLibrary::GetMyPlayerState(this))
	{
		MyPlayerState->OnPlayerNameChanged.AddDynamic(this, &ThisClass::OnPlayerNameChanged);
		OnPlayerNameChanged(MyPlayerState->GetPlayerFNameCustom());

		SetDefaultPlayerMeshData();
	}

	UpdateCollisionObjectType();
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
		FCells PlayerCells;
		AGeneratedMap::Get().IntersectCellsByTypes(PlayerCells, TO_FLAG(EAT::Player), true);
		CharacterIDInternal = PlayerCells.Num() - 1;
		OnRep_CharacterID();
	}

	static const FName DefaultAIName(TEXT("AI"));
	OnPlayerNameChanged(DefaultAIName);

	// Spawn or destroy controller of specific ai with enabled visualization
#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld] only
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
}

// Is overriden to handle the client login when is set new player state
void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	SetDefaultPlayerMeshData();
}

// Triggers when this player character starts something overlap.
void APlayerCharacter::OnPlayerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AItemActor* OverlappedItem = Cast<AItemActor>(OtherActor);
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

	// Uninitialize item
	OverlappedItem->ResetItemType();
}

// Event triggered when the bomb has been explicitly destroyed.
void APlayerCharacter::OnBombDestroyed(AActor* DestroyedBomb)
{
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

// Update player name on a 3D widget component
void APlayerCharacter::OnPlayerNameChanged_Implementation(FName NewName)
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
	if (!HasAuthority()
	    || CharacterIDInternal < 0)
	{
		return;
	}

	// Try possess the player
	if (AMyPlayerController* MyPC = USingletonLibrary::GetMyPlayerController(CharacterIDInternal))
	{
		MyPC->Possess(this);
		return;
	}

	// Spawn AI if is needed
	UWorld* World = GetWorld();
	if (World
	    && !IS_VALID(MyAIControllerInternal))
	{
		MyAIControllerInternal = World->SpawnActor<AMyAIController>(AIControllerClass, GetActorTransform());
	}

	// Try possess the AI
	if (MyAIControllerInternal)
	{
		MyAIControllerInternal->Possess(this);
	}
}

// Is called on game mode post login to handle character logic when new player is connected
void APlayerCharacter::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	TryPossessController();

	SetDefaultPlayerMeshData();
}

// Apply default mesh for the local player
void APlayerCharacter::SetDefaultPlayerMeshData()
{
	AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>();
	if (!MyPlayerState)
	{
		return;
	}

	FCustomPlayerMeshData CustomPlayerMeshData = MyPlayerState->GetCustomPlayerMeshData();
	if (CustomPlayerMeshData.IsValid())
	{
		InitMySkeletalMesh(CustomPlayerMeshData);
	}
	else
	{
		// Set default custom player mesh
		CustomPlayerMeshData.PlayerRow = UPlayerDataAsset::Get().GetRowByLevelType<UPlayerRow>(USingletonLibrary::GetLevelType());
		MyPlayerState->SetCustomPlayerMeshData(CustomPlayerMeshData);
	}
}

// Is called on server and clients to apply the characterID-dependent logic for this character
void APlayerCharacter::OnRep_CharacterID()
{
	if (CharacterIDInternal == INDEX_NONE)
	{
		return;
	}

	const UPlayerDataAsset& PlayerDataAsset = UPlayerDataAsset::Get();

	// Update mesh
	FCustomPlayerMeshData CustomPlayerMeshData = FCustomPlayerMeshData::Empty;
	const int32 MeshesNum = PlayerDataAsset.GetRowsNum();
	if (MeshesNum > 0)
	{
		const int32 LevelType = 1 << (CharacterIDInternal % MeshesNum);
		if (UPlayerRow* Row = Cast<UPlayerRow>(PlayerDataAsset.GetRowByLevelType(TO_ENUM(ELevelType, LevelType))))
		{
			CustomPlayerMeshData.PlayerRow = Row;
			CustomPlayerMeshData.SkinIndex = FMath::RandHelper(Row->GetMaterialInstancesDynamicNum());
			InitMySkeletalMesh(CustomPlayerMeshData);
		}
	}

	// Set a nameplate material
	if (ensureMsgf(NameplateMeshInternal, TEXT("ASSERT: 'NameplateMeshComponent' is not valid")))
	{
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
}
