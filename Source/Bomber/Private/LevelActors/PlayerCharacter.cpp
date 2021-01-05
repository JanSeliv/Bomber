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
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Default constructor
UPlayerDataAsset::UPlayerDataAsset()
{
	ActorTypeInternal = EAT::Player;
	RowClassInternal = UPlayerRow::StaticClass();
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
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90.f), FRotator(0, -90.f, 0));

	// Initialize the nameplate mesh component
	NameplateMeshInternal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NameplateMeshComponent"));
	NameplateMeshInternal->SetupAttachment(RootComponent);
	NameplateMeshInternal->SetRelativeLocation(FVector(-60.f, 0.f, 150.f));
	NameplateMeshInternal->SetRelativeScale3D(FVector(1.75f, 1.f, 1.f));
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
	AController* OwnedController = GetController();
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap                                   // The Level Map is not accessible
	    || !IsValid(MapComponentInternal)           // The Map Component is not valid or transient
	    || PowerupsInternal.FireN <= 0              // Null length of explosion
	    || PowerupsInternal.BombN <= 0              // No more bombs
	    || USingletonLibrary::IsEditorNotPieWorld() // Should not spawn bomb in PIE
	    || !OwnedController                         // controller is not valid
	    || OwnedController->IsMoveInputIgnored())   // controller is blocked
	{
		return;
	}

	// Spawn bomb
	auto BombActor = Cast<ABombActor>(LevelMap->SpawnActorByType(EAT::Bomb, MapComponentInternal->Cell));
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
		MeshComp->SetAnimInstanceClass(MapComponentInternal->GetDataAssetChecked<UPlayerDataAsset>()->AnimBlueprintClass);
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
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	// Setup timer handle to update a player location on the level map (initialized being paused)
	if (const UGeneratedMapDataAsset* LevelsDataAsset = USingletonLibrary::GetLevelsDataAsset())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TWeakObjectPtr<ThisClass> WeakThis(this);
		TimerManager.SetTimer(UpdatePositionHandleInternal, [WeakThis]
		{
			AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
			if (const APlayerCharacter* PlayerCharacter = WeakThis.Get())
			{
				LevelMap->SetNearestCell(PlayerCharacter->MapComponentInternal);
			}
		}, LevelsDataAsset->GetTickInterval(), true, KINDA_SMALL_NUMBER);
		TimerManager.PauseTimer(UpdatePositionHandleInternal);
	}
}

// Called when an instance of this class is placed (in editor) or spawned
void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IS_TRANSIENT(this)                // This actor is transient
	    || !IsValid(MapComponentInternal) // Is not valid for map construction
	    || !LevelMap)                     // the level map is not valid or transient
	{
		return;
	}

	// Construct the actor's map component
	MapComponentInternal->OnConstruction();

	// Set ID
	if (CharacterIDInternal == INDEX_NONE)
	{
		FCells PlayerCells;
		LevelMap->IntersectCellsByTypes(PlayerCells, TO_FLAG(EAT::Player), true);
		CharacterIDInternal = PlayerCells.Num() - 1;
	}

	// Override mesh
	const ULevelActorDataAsset* PlayerDataAsset = MapComponentInternal->GetActorDataAsset();
	const int32 MeshesNum = PlayerDataAsset ? PlayerDataAsset->GetRowsNum() : 0;
	if (MeshesNum)
	{
		const int32 LevelType = 1 << (CharacterIDInternal % MeshesNum);
		const ULevelActorRow* Row = PlayerDataAsset->GetRowByLevelType(TO_ENUM(ELevelType, LevelType));
		MapComponentInternal->SetMeshByRow(Row, GetMesh());
	}

	// Update mesh if chosen
	if (!CharacterIDInternal) // is player
	{
		if (const AMyPlayerState* MyPlayerState = USingletonLibrary::GetMyPlayerState(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
		{
			MapComponentInternal->SetMeshByRow(MyPlayerState->GetChosenPlayerRaw());
		}
	}

	// Set a nameplate material
	if (ensureMsgf(NameplateMeshInternal, TEXT("ASSERT: 'NameplateMeshComponent' is not valid")))
	{
		const TArray<UMaterialInterface*>& NameplateMaterials = MapComponentInternal->GetDataAssetChecked<UPlayerDataAsset>()->NameplateMaterials;
		const int32 NameplateMeshesNum = NameplateMaterials.Num();
		if (NameplateMeshesNum > 0)
		{
			const int32 MaterialNo = CharacterIDInternal < NameplateMeshesNum ? CharacterIDInternal : CharacterIDInternal % NameplateMeshesNum;
			if (NameplateMaterials.IsValidIndex(MaterialNo))
			{
				NameplateMeshInternal->SetMaterial(0, NameplateMaterials[MaterialNo]);
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
		if (MapComponentInternal->bShouldShowRenders == false)
		{
			if (MyAIControllerInternal) MyAIControllerInternal->Destroy();
		}
		else if (MyAIControllerInternal == nullptr) // Is a bot with debug visualization and AI controller is not created yet
		{
			SpawnDefaultController();
			if (GetController()) GetController()->bIsEditorOnlyActor = true;
		}
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveUpDown", this, &APlayerCharacter::OnMoveUpDown);
	PlayerInputComponent->BindAxis("MoveRightLeft", this, &APlayerCharacter::OnMoveRightLeft);
	PlayerInputComponent->BindAction("SpaceEvent", IE_Pressed, this, &APlayerCharacter::SpawnBomb);
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

// Triggers when this player character starts something overlap.
void APlayerCharacter::OnPlayerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AItemActor* OverlappedItem = Cast<AItemActor>(OtherActor);
	const EItemType ItemType = OverlappedItem ? OverlappedItem->GetItemType() : EItemType::None;
	if (ItemType == EItemType::None) // item is not valid
	{
		return;
	}

	switch (ItemType)
	{
		case EItemType::Skate:
		{
			const int32 SkateN = ++PowerupsInternal.SkateN * 100.F + 500.F;
			UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
			if (MovementComponent    //  is accessible
			    && SkateN <= 1000.F) // is lower than the max speed value (5x skate items)
			{
				MovementComponent->MaxWalkSpeed = SkateN;
			}
			break;
		}
		case EItemType::Bomb:
		{
			PowerupsInternal.BombN++;
			break;
		}
		case EItemType::Fire:
		{
			PowerupsInternal.FireN++;
			break;
		}
		default: break;
	}

	// Uninitialize item
	OverlappedItem->ResetItemType();
}

// Event triggered when the bomb has been explicitly destroyed.
void APlayerCharacter::OnBombDestroyed(AActor* DestroyedBomb)
{
	PowerupsInternal.BombN++;
}

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
		default: break;
	}
}

//
void APlayerCharacter::UpdateNickname_Implementation() const
{
	// BP implementation
	// ...
}
