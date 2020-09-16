// Copyright 2020 Yevhenii Selivanov.

#include "LevelActors/PlayerCharacter.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "LevelActors/BombActor.h"
#include "MapComponent.h"
#include "MyAIController.h"
#include "SingletonLibrary.h"
#include "ItemActor.h"
#include "GameFramework/MyGameStateBase.h"
//---
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Default constructor
UPlayerDataAsset::UPlayerDataAsset()
{
	ActorTypeInternal = EAT::Player;
}

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to don't call Tick()
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Set the default AI controller class
	AIControllerClass = AMyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90.f), FRotator(0, -90.f, 0));

	// Initialize the nameplate mesh component
	NameplateMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NameplateMeshComponent"));
	NameplateMeshComponent->SetupAttachment(RootComponent);
	NameplateMeshComponent->SetRelativeLocation(FVector(-60.f, 0.f, 150.f));
	NameplateMeshComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	NameplateMeshComponent->SetRelativeScale3D(FVector(1.75f, 1.f, 1.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameplateMeshFinder(TEXT("/Engine/BasicShapes/Plane"));
	if (NameplateMeshFinder.Succeeded())
	{
		NameplateMeshComponent->SetStaticMesh(NameplateMeshFinder.Object);
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
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap                                     // The Level Map is not accessible
	    || !IsValid(MapComponent)            // The Map Component is not valid or transient
	    || PowerupsInternal.FireN <= 0               // Null length of explosion
	    || PowerupsInternal.BombN <= 0               // No more bombs
	    || USingletonLibrary::IsEditorNotPieWorld()) // Should not spawn bomb in PIE
	{
		return;
	}

	// Spawn bomb
	auto BombActor = Cast<ABombActor>(LevelMap->SpawnActorByType(EAT::Bomb, MapComponent->Cell));
	if (BombActor) // can return nullptr if the cell is not free
	{
		// Updating explosion cells
		PowerupsInternal.BombN--;

		// Init Bomb
		ABombActor::FOnBombDestroyed OnBombDestroyed;
		OnBombDestroyed.BindDynamic(this, &ThisClass::OnBombDestroyed);
		BombActor->InitBomb(OnBombDestroyed, PowerupsInternal.FireN, CharacterID_);
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
	if (!World)
	{
		return;
	}

	// Set the animation
	if (GetMesh()->GetAnimInstance() == nullptr	 // Is not created yet
		&& MyAnimClass != nullptr)				 // The animation class is set
	{
		GetMesh()->SetAnimInstanceClass(MyAnimClass);
	}

	// Posses the controller
	if (CharacterID_ == 0)	// Is the player (not AI)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
		if (PlayerController)
		{
			PlayerController->Possess(this);
		}
	}
	else								// has AI controller
		if (!IS_VALID(MyAIController))	// was not spawned early
	{
		MyAIController = World->SpawnActor<AMyAIController>(AIControllerClass, GetActorTransform());
		MyAIController->Possess(this);
	}

	OnActorBeginOverlap.AddDynamic(this, &APlayerCharacter::OnPlayerBeginOverlap);

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	// Setup timer handle to update a player location on the level map (initialized being paused)
	if(const UGeneratedMapDataAsset* LevelsDataAsset = USingletonLibrary::GetLevelsDataAsset())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TWeakObjectPtr<ThisClass> WeakThis(this);
		TimerManager.SetTimer(UpdatePositionHandleInternal, [WeakThis]
		{
			if (AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap())
			{
				LevelMap->SetNearestCell(WeakThis.IsValid() ? WeakThis.Get()->MapComponent : nullptr);
			}
		}, LevelsDataAsset->GetTickInterval(), true);
		TimerManager.PauseTimer(UpdatePositionHandleInternal);
	}
}

// Called when an instance of this class is placed (in editor) or spawned
void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IS_TRANSIENT(this)        // This actor is transient
	    || !IsValid(MapComponent) // Is not valid for map construction
	    || !LevelMap)             // the level map is not valid or transient
	{
		return;
	}

	// Setting an ID to the player character as his position in the array
	// FCells PlayersCells;
	// LevelMap->IntersectCellsByTypes(PlayersCells, TO_FLAG(EAT::Player));
	// CharacterID_ = PlayersCells.FindId(MapComponent->Cell).AsInteger();
	// ensureMsgf(CharacterID_ != INDEX_NONE, TEXT("The character was not found on the Level Map"));

	CharacterID_= LevelMap->GetCharactersNum();

	// Construct the actor's map component
	const ELevelType LevelType = TO_ENUM(ELevelType, (1 << CharacterID_) % TO_FLAG(ELevelType::Max));
	MapComponent->OnComponentConstruct(GetMesh(), FLevelActorMeshRow(LevelType));

	// Set a nameplate material
	if (ensureMsgf(NameplateMeshComponent, TEXT("ASSERT: 'NameplateMeshComponent' is not valid")))
	{
		const TArray<UMaterialInterface*>& NameplateMaterials = MapComponent->GetDataAssetChecked<UPlayerDataAsset>()->NameplateMaterials;
		const int32 NameplateMeshesNum = NameplateMaterials.Num();
		if (NameplateMeshesNum > 0)
		{
			const int32 MaterialNo = CharacterID_ < NameplateMeshesNum ? CharacterID_ : CharacterID_ % NameplateMeshesNum;
			if (NameplateMaterials.IsValidIndex(MaterialNo))
			{
				NameplateMeshComponent->SetMaterial(0, NameplateMaterials[MaterialNo]);
			}
		}
	}

	// Spawn or destroy controller of specific ai with enabled visualization
#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld()  // [IsEditorNotPieWorld] only
		&& CharacterID_ > 0)					  // Is a bot
	{
		MyAIController = Cast<AMyAIController>(GetController());
		if (MapComponent->bShouldShowRenders == false)
		{
			if (MyAIController) MyAIController->Destroy();
		}
		else								// Is a bot with debug visualization
			if (MyAIController == nullptr)	// AI controller is not created yet
		{
			SpawnDefaultController();
			if (GetController()) GetController()->bIsEditorOnlyActor = true;
		}
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	// Rotate this character
	const float YawRotation = USingletonLibrary::GetLevelMap()->GetActorRotation().Yaw - 90.f;
	SetActorRotation(FRotator(0.f, YawRotation, 0.f));
	USingletonLibrary::PrintToLog(this, "OnConstruction \t New rotation:", GetActorRotation().ToString());
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
void APlayerCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
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

void APlayerCharacter::OnGameStateChanged(ECurrentGameState CurrentGameState)
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
