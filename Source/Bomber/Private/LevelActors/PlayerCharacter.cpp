// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelActors/PlayerCharacter.h"

#include "Animation/AnimInstance.h"			   //UAnimInstance
#include "Components/SkeletalMeshComponent.h"  // USkeletalMesh
#include "Components/StaticMeshComponent.h"	// UStaticMeshComponent
#include "Components/TextRenderComponent.h"	//UTextRenderComponent
#include "UObject/ConstructorHelpers.h"		   // ConstructorHelpers

#include "LevelActors/BombActor.h"
#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "MyAIController.h"
#include "MyGameInstance.h"
#include "SingletonLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to don't call Tick()
	PrimaryActorTick.bCanEverTick = false;

	// Set the default AI controller class
	AIControllerClass = AMyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90.f), FRotator(0, -90.f, 0));

	// Find skeletal meshes
	static TArray<ConstructorHelpers::FObjectFinder<USkeletalMesh>> SkeletalMeshFinderArray{
		TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/Phoenix/Meshes/IggyScorch_Phoenix"),
		TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/MechaTerror/Meshes/IggyScorch_MechaTerror"),
		TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/JingleBombs/Meshes/IggyScorch_JingleBombs"),
		TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Skins/Fireball/Meshes/IggyScorch_Fireball")};
	for (int32 i = 0; i < SkeletalMeshFinderArray.Num(); ++i)
	{
		if (SkeletalMeshFinderArray[i].Succeeded())
		{
			SkeletalMeshes.Add(SkeletalMeshFinderArray[i].Object);
			if (i == 0) GetMesh()->SetSkeletalMesh(SkeletalMeshFinderArray[i].Object);  // preview
		}
	}

	// Set the animation
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimationFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/IggyScorch_AnimBP"));
	if (AnimationFinder.Succeeded())  // The animation was found
	{
		MyAnimClass = AnimationFinder.Class;
	}

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
		TEXT("/Game/Bomber/Assets/MI_NamePlates/MI_NamePlateYellow"),
		TEXT("/Game/Bomber/Assets/MI_NamePlates/MI_NamePlateBlue"),
		TEXT("/Game/Bomber/Assets/MI_NamePlates/MI_NamePlateWhite"),
		TEXT("/Game/Bomber/Assets/MI_NamePlates/MI_NamePlatePink")};
	for (int32 i = 0; i < MaterialsFinderArray.Num(); ++i)
	{
		if (MaterialsFinderArray[i].Succeeded())
		{
			NameplateMaterials.Add(MaterialsFinderArray[i].Object);
		}
	}

	// Initialize the nickname text render component
	NicknameTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NicknameTextRender"));
	NicknameTextRender->SetupAttachment(NameplateMeshComponent);
	NicknameTextRender->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	NicknameTextRender->SetRelativeRotation(FRotator(90.f, -90.f, 180.f));
	NicknameTextRender->SetHorizontalAlignment(EHTA_Center);
	NicknameTextRender->SetVerticalAlignment(EVRTA_TextCenter);
	NicknameTextRender->SetTextRenderColor(FColor::Black);
	NicknameTextRender->SetWorldSize(56.f);
	NicknameTextRender->SetText(DEFAULT_NICKNAME);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SpaceEvent", IE_Pressed, this, &APlayerCharacter::SpawnBomb);
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set the animation
	if (GetMesh()->GetAnimInstance() == nullptr  // Is not created yet
		&& MyAnimClass != nullptr)				 // The animation class is set
	{
		GetMesh()->SetAnimInstanceClass(MyAnimClass);
	}

	// Posses the controller
	if (CharacterID_ == 0)  // Is the player (not AI)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			PlayerController->Possess(this);
		}
	}
	else								// has AI controller
		if (!IS_VALID(MyAIController))  // was not spawned early
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

	// Set the character ID

	// The character is initialized for the first time.
	if (CharacterID_ == INDEX_NONE)
	{
		CharacterID_ = LevelMap->GetCharactersNum() - 1;
		check(CharacterID_ > INDEX_NONE && "The LevelMap is valid, by no characters in the array");
	}

	// Set a character skeletal mesh
	if (GetMesh())
	{
		const int32 SkeletalNo = CharacterID_ < SkeletalMeshes.Num() ? CharacterID_ : CharacterID_ % SkeletalMeshes.Num();
		GetMesh()->SetSkeletalMesh(SkeletalMeshes[SkeletalNo]);
	}

	// Set a nameplate material
	if (NameplateMeshComponent)
	{
		const int32 MaterialNo = CharacterID_ < NameplateMaterials.Num() ? CharacterID_ : CharacterID_ % NameplateMaterials.Num();
		NameplateMeshComponent->SetMaterial(0, NameplateMaterials[MaterialNo]);
	}

	// Set the nickname
	if (NicknameTextRender)
	{
		if (CharacterID_ == 0)  // is a player
		{
			UMyGameInstance* MyGameInstance = USingletonLibrary::GetMyGameInstance(this);
			NicknameTextRender->SetText(MyGameInstance ? MyGameInstance->Nickname : DEFAULT_NICKNAME);
		}
		else  // is a bot
		{
			NicknameTextRender->SetText(FText::FromString(TEXT("AI")));
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
			if (MyAIController == nullptr)  // AI controller is not created yet
		{
			SpawnDefaultController();
			if (GetController()) GetController()->bIsEditorOnlyActor = true;
		}
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]

	// Rotate this character
	const float YawRotation = USingletonLibrary::GetLevelMap()->GetActorRotation().Yaw - 90.f;
	SetActorRotation(FRotator(0.f, YawRotation, 0.f));
	USingletonLibrary::PrintToLog(this, "OnConstruction \t New rotation:", GetActorRotation().ToString());
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
	auto Bomb = Cast<ABombActor>(LevelMap->SpawnActorByType(EActorTypeEnum::Bomb, MapComponent->GetCell()));

	// Update material of mesh
	if (Bomb != nullptr)
	{
		Bomb->InitializeBombProperties(Powerups_.BombN, Powerups_.FireN, CharacterID_);
	}
}