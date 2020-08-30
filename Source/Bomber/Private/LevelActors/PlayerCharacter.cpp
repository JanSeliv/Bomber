// Copyright 2020 Yevhenii Selivanov.

#include "LevelActors/PlayerCharacter.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "LevelActors/BombActor.h"
#include "MapComponent.h"
#include "MyAIController.h"
#include "SingletonLibrary.h"
//---
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

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

	// Find skeletal meshes
	// @TODO Get skeletal meshes from the Singleton Library
	// static TArray<ConstructorHelpers::FObjectFinder<USkeletalMesh>> SkeletalMeshFinderArray{
	// 	TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/Phoenix/Meshes/IggyScorch_Phoenix"),
	// 	TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/MechaTerror/Meshes/IggyScorch_MechaTerror"),
	// 	TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/JingleBombs/Meshes/IggyScorch_JingleBombs"),
	// 	TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/Fireball/Meshes/IggyScorch_Fireball")};
	// for (int32 i = 0; i < SkeletalMeshFinderArray.Num(); ++i)
	// {
	// 	if (SkeletalMeshFinderArray[i].Succeeded())
	// 	{
	// 		SkeletalMeshes.Emplace(SkeletalMeshFinderArray[i].Object);
	// 		if (i == 0) GetMesh()->SetSkeletalMesh(SkeletalMeshFinderArray[i].Object);	// preview
	// 	}
	// }

	// Set the animation
	// @TODO Get the animation blueprint from the Singleton Library
	// static ConstructorHelpers::FClassFinder<UAnimInstance> AnimationFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/IggyScorch_AnimBP"));
	// if (AnimationFinder.Succeeded())  // The animation was found
	// {
	// 	MyAnimClass = AnimationFinder.Class;
	// }

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

	// Find nameplate materials
	static TArray<ConstructorHelpers::FObjectFinder<UMaterialInterface>> MaterialsFinderArray{
		TEXT("/Game/Bomber/Materials/MI_Nameplates/MI_Nameplate_Yellow"),
		TEXT("/Game/Bomber/Materials/MI_Nameplates/MI_Nameplate_Blue"),
		TEXT("/Game/Bomber/Materials/MI_Nameplates/MI_Nameplate_White"),
		TEXT("/Game/Bomber/Materials/MI_Nameplates/MI_Nameplate_Pink")};
	for (int32 i = 0; i < MaterialsFinderArray.Num(); ++i)
	{
		if (MaterialsFinderArray[i].Succeeded())
		{
			NameplateMaterials.Emplace(MaterialsFinderArray[i].Object);
		}
	}
}

// Finds and rotates the self at the current character's location to point at the specified location.
void APlayerCharacter::RotateToLocation(const FVector& Location, bool bShouldInterpolate) const
{
	UWorld* World = GetWorld();
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!World	//
		|| !MeshComponent)
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

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set the animation
	if (GetMesh()->GetAnimInstance() == nullptr	 // Is not created yet
		&& MyAnimClass != nullptr)				 // The animation class is set
	{
		GetMesh()->SetAnimInstanceClass(MyAnimClass);
	}

	// Posses the controller
	if (CharacterID_ == 0)	// Is the player (not AI)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			PlayerController->Possess(this);
		}
	}
	else								// has AI controller
		if (!IS_VALID(MyAIController))	// was not spawned early
	{
		MyAIController = GetWorld()->SpawnActor<AMyAIController>(AIControllerClass, GetActorTransform());
		MyAIController->Possess(this);
	}
}

// Called when an instance of this class is placed (in editor) or spawned
void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IS_TRANSIENT(this)		   // This actor is transient
		|| !IsValid(MapComponent)  // Is not valid for map construction
		|| !LevelMap)			   // the level map is not valid or transient
	{
		return;
	}
	// Construct the actor's map component
	MapComponent->OnMapComponentConstruction();

	// Setting an ID to the player character as his position in the array
	FCells PlayersCells;
	LevelMap->IntersectCellsByTypes(PlayersCells, TO_FLAG(EActorType::Player));
	CharacterID_ = PlayersCells.FindId(MapComponent->Cell).AsInteger();
	check(CharacterID_ != INDEX_NONE && "The character was not found on the Level Map");

	// Set a character skeletal mesh
	const int32 SkeletalMeshesNum = SkeletalMeshes.Num();
	if (GetMesh()
		&& SkeletalMeshesNum)
	{
		const int32 SkeletalNo = CharacterID_ < SkeletalMeshesNum ? CharacterID_ : CharacterID_ % SkeletalMeshesNum;
		if(SkeletalMeshes.IsValidIndex(SkeletalNo))
		{
			GetMesh()->SetSkeletalMesh(SkeletalMeshes[SkeletalNo]);
		}
	}

	// Set a nameplate material
	if (NameplateMeshComponent)
	{
		const int32 MaterialNo = CharacterID_ < NameplateMaterials.Num() ? CharacterID_ : CharacterID_ % NameplateMaterials.Num();
		NameplateMeshComponent->SetMaterial(0, NameplateMaterials[MaterialNo]);
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

// Spawns bomb on character position
void APlayerCharacter::SpawnBomb()
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (LevelMap == nullptr							  // The Level Map is not accessible
		|| IsValid(MapComponent) == false			  // The Map Component is not valid or transient
		|| Powerups_.FireN <= 0						  // Null length of explosion
		|| Powerups_.BombN <= 0						  // No more bombs
		|| USingletonLibrary::IsEditorNotPieWorld())  // Should not spawn bomb in PIE
	{
		return;
	}

	// Spawn bomb
	auto BombActor = Cast<ABombActor>(LevelMap->SpawnActorByType(EActorType::Bomb, MapComponent->Cell));
	if (ensureMsgf(BombActor, TEXT("ASSERT: 'BombActor' is not valid")))
	{
		// Updating explosion cells
		Powerups_.BombN--;
		FOnBombDestroyed OnBombDestroyed;
		OnBombDestroyed.BindDynamic(this, &ThisClass::OnBombDestroyed);
		BombActor->InitBomb(OnBombDestroyed, Powerups_.FireN, CharacterID_);
	}
}

// Event triggered when the bomb has been explicitly destroyed.
void APlayerCharacter::OnBombDestroyed(AActor* DestroyedBomb)
{
	Powerups_.BombN++;
}
