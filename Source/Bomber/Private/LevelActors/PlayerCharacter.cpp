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
//---
#include "Animation/AnimInstance.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2DArray.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

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
	FProperty* Property = PropertyChangedEvent.Property;
	if (Property
	    && Property->IsA<FObjectProperty>()
	    && PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet
	    && Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MaterialInstanceInternal))
	{
		// Force recreation of dynamic material instances
		MaterialInstancesDynamicInternal.Empty();
		TryCreateDynamicMaterials();
	}
}
#endif	//WITH_EDITOR

//
void UPlayerRow::PostLoad()
{
	Super::PostLoad();

	TryCreateDynamicMaterials();
}

//
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
	const int32 SkinTexturesNum = Texture2DArray ? Texture2DArray->GetNumSlices() : 0;
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
		const int32 SkinPosition = MaterialInstancesDynamicInternal.Add(MaterialInstanceDynamic);
		MaterialInstanceDynamic->SetScalarParameterValue(SkinIndexParameterName, SkinPosition);
	}
}

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
	// Set this character to don't call Tick()
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Set the default AI controller class
	AIControllerClass = AMyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
		static const FVector MeshRelativeLocation(0, 0, -90.f);
		SkeletalMeshComponent->SetRelativeLocation_Direct(MeshRelativeLocation);
		static const FRotator MeshRelativeRotation(0, -90.f, 0);
		SkeletalMeshComponent->SetRelativeRotation_Direct(MeshRelativeRotation);
	}

	// Initialize the nameplate mesh component
	NameplateMeshInternal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NameplateMeshComponent"));
	NameplateMeshInternal->SetupAttachment(RootComponent);
	static const FVector NameplateRelativeLocation(-60.f, 0.f, 150.f);
	NameplateMeshInternal->SetRelativeLocation_Direct(NameplateRelativeLocation);
	static const FVector NameplateRelativeScale(1.75f, 1.f, 1.f);
	NameplateMeshInternal->SetRelativeScale3D_Direct(NameplateRelativeScale);

	// Disable gravity
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->GravityScale = 0.f;
	}
}

// Finds and rotates the self at the current character's location to point at the specified location.
void APlayerCharacter::RotateToLocation(const FVector& Location, bool bShouldInterpolate) const
{
	UWorld* World = GetWorld();
	USkeletalMeshComponent* MeshComponent = GetMesh();
	AController* OwnedController = GetController();
	if (!World
	    || !MeshComponent
	    || !OwnedController
	    || OwnedController->IsMoveInputIgnored())
	{
		return;
	}

	FRotator TargetRotation = FRotator::ZeroRotator;
	TargetRotation.Yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Location).Yaw - 90.F;

	FRotator NewRotation;
	if (bShouldInterpolate)
	{
		NewRotation = UKismetMathLibrary::RInterpTo(
			MeshComponent->GetComponentRotation(), TargetRotation, World->GetDeltaSeconds() * 10.F, 1.F);
	}
	else
	{
		NewRotation = TargetRotation;
	}

	MeshComponent->SetWorldRotation(NewRotation);
}

// Spawns bomb on character position
void APlayerCharacter::SpawnBomb()
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
	auto BombActor = Cast<ABombActor>(AGeneratedMap::Get().SpawnActorByType(EAT::Bomb, MapComponentInternal->Cell));
	if (BombActor) // can return nullptr if the cell is not free
	{
		// Updating explosion cells
		PowerupsInternal.BombN--;

		// Init Bomb
		ABombActor::FOnBombDestroyed OnBombDestroyed;
		OnBombDestroyed.BindDynamic(this, &ThisClass::OnBombDestroyed);
		BombActor->InitBomb(OnBombDestroyed, PowerupsInternal.FireN, CharacterIDInternal);
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

// Update player name on a 3D widget component
void APlayerCharacter::UpdateNickname_Implementation() const
{
	// BP implementation
	// ...
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
	if (!World
	    || !MapComponentInternal)
	{
		return;
	}

	// Set the animation
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const TSubclassOf<UAnimInstance> AnimInstanceClass = UPlayerDataAsset::Get().GetAnimInstanceClass();
		MeshComp->SetAnimInstanceClass(AnimInstanceClass);
	}

	// Posses the controller
	if (!CharacterIDInternal) // Is the player (not AI)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
		if (PlayerController)
		{
			PlayerController->Possess(this);
		}
	}
	else if (!IS_VALID(MyAIControllerInternal)) // has AI controller and was not spawned early
	{
		MyAIControllerInternal = World->SpawnActor<AMyAIController>(AIControllerClass, GetActorTransform());
		MyAIControllerInternal->Possess(this);
	}

	OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerBeginOverlap);

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	// Setup timer handle to update a player location on the level map (initialized being paused)
	FTimerManager& TimerManager = World->GetTimerManager();
	TWeakObjectPtr<ThisClass> WeakThis(this);
	TimerManager.SetTimer(UpdatePositionHandleInternal, [WeakThis]
	{
		if (const APlayerCharacter* PlayerCharacter = WeakThis.Get())
		{
			AGeneratedMap::Get().SetNearestCell(PlayerCharacter->MapComponentInternal);
		}
	}, UGeneratedMapDataAsset::Get().GetTickInterval(), true, KINDA_SMALL_NUMBER);
	TimerManager.PauseTimer(UpdatePositionHandleInternal);

	UpdateNickname();
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
	MapComponentInternal->OnConstruction();

	// Set ID
	if (CharacterIDInternal == INDEX_NONE)
	{
		FCells PlayerCells;
		AGeneratedMap::Get().IntersectCellsByTypes(PlayerCells, TO_FLAG(EAT::Player), true);
		CharacterIDInternal = PlayerCells.Num() - 1;
	}

	// Update mesh
	FCustomPlayerMeshData CustomPlayerMeshData;
	const AMyPlayerState* MyPlayerState = !CharacterIDInternal ? USingletonLibrary::GetCurrentPlayerState() : nullptr;
	if (MyPlayerState)
	{
		CustomPlayerMeshData = MyPlayerState->GetCustomPlayerMeshData();
	}
	else // is bot
	{
		const ULevelActorDataAsset* PlayerDataAsset = MapComponentInternal->GetActorDataAsset();
		const int32 MeshesNum = PlayerDataAsset ? PlayerDataAsset->GetRowsNum() : 0;
		if (MeshesNum)
		{
			const int32 LevelType = 1 << (CharacterIDInternal % MeshesNum);
			if (UPlayerRow* Row = Cast<UPlayerRow>(PlayerDataAsset->GetRowByLevelType(TO_ENUM(ELevelType, LevelType))))
			{
				CustomPlayerMeshData.PlayerRow = Row;
				CustomPlayerMeshData.SkinIndex = FMath::RandHelper(Row->GetMaterialInstancesDynamicNum());
			}
		}
	}
	InitMySkeletalMesh(CustomPlayerMeshData);

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

		UpdateNickname();
	}

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

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!ensureMsgf(PlayerInputComponent, TEXT("ASSERT: 'PlayerInputComponent' is not valid")))
	{
		return;
	}

	// Do not consume added input
	auto SetInput = [](FInputBinding& InputRef) { InputRef.bConsumeInput = false; };

	static const FName MoveUpDownName = GET_FUNCTION_NAME_CHECKED(ThisClass, MoveUpDown);
	SetInput(PlayerInputComponent->BindAxis(MoveUpDownName, this, &ThisClass::MoveUpDown));

	static const FName MoveRightLeftName = GET_FUNCTION_NAME_CHECKED(ThisClass, MoveRightLeft);
	SetInput(PlayerInputComponent->BindAxis(MoveRightLeftName, this, &ThisClass::MoveRightLeft));

	static const FName SpawnBombName = GET_FUNCTION_NAME_CHECKED(ThisClass, SpawnBomb);
	SetInput(PlayerInputComponent->BindAction(SpawnBombName, IE_Pressed, this, &ThisClass::SpawnBomb));
}

// Adds the movement input along the given world direction vector.
void APlayerCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue/* = 1.f*/, bool bForce/* = false*/)
{
	if (ScaleValue)
	{
		// Move the character
		Super::AddMovementInput(WorldDirection, ScaleValue, bForce);

		// Rotate the character
		RotateToLocation(GetActorLocation() + ScaleValue * WorldDirection, true);
	}
}

// Move the player character by the forward vector
void APlayerCharacter::MoveUpDown(float ScaleValue)
{
	AddMovementInput(GetActorRightVector(), ScaleValue);
}

void APlayerCharacter::MoveRightLeft(float ScaleValue)
{
	AddMovementInput(GetActorForwardVector(), ScaleValue);
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

// Called when the current game state was changed
void APlayerCharacter::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		case ECurrentGameState::GameStarting:
		{
			TimerManager.PauseTimer(UpdatePositionHandleInternal);
			break;
		}
		case ECurrentGameState::InGame:
		{
			TimerManager.UnPauseTimer(UpdatePositionHandleInternal);
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
