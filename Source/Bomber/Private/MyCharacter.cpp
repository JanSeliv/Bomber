// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"

#include "Animation/AnimBlueprint.h"		   // UAnimBlueprint
#include "Components/SkeletalMeshComponent.h"  // USkeletalMesh
#include "Components/StaticMeshComponent.h"	// UStaticMeshComponent
#include "Components/TextRenderComponent.h"	//UTextRenderComponent
#include "UObject/ConstructorHelpers.h"		   // ConstructorHelpers

#include "BombActor.h"
#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
		}
	}

	// Set the animation
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimationFinder(TEXT("AnimBlueprint'/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/IggyScorch_AnimBP.IggyScorch_AnimBP'"));
	if (AnimationFinder.Succeeded())  // The animation was found
	{
		GetMesh()->AnimClass = AnimationFinder.Object->GeneratedClass;
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
	NicknameTextRender->SetText(TEXT("Player"));
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterID_ == 0)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			PlayerController->Possess(this);
		}
	}
}

void AMyCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!IsValid(USingletonLibrary::GetLevelMap())  //
		|| IS_VALID(MapComponent) == false)			// this component is not valid for owner construction
	{
		return;
	}

	// Construct the actor's map component
	MapComponent->OnMapComponentConstruction();

	// Set the character ID
	CharacterID_ = USingletonLibrary::GetLevelMap()->CharactersOnMap.IndexOfByKey(this);

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
		NicknameTextRender->SetText(CharacterID_ == 0 ? TEXT("Player") : TEXT("AI"));
	}

	// The bots construction
	if (CharacterID_ > 0)  // Is a bot
	{
		if (GetController() == nullptr)  // The ai controller is not created yet
		{
			SpawnDefaultController();
			if (GetController()) GetController()->Possess(this);
		}
	}

	// Rotate this character
	const float YawRotation = USingletonLibrary::GetLevelMap()->GetActorRotation().Yaw - 90.f;
	SetActorRotation(FRotator(0.f, YawRotation, 0.f));
	USingletonLibrary::PrintToLog(this, "OnConstruction \t New rotation:", GetActorRotation().ToString());
}

void AMyCharacter::Destroyed()
{
	UWorld* const World = GetWorld();
	if (World != nullptr							  // World is not null
		&& IsValid(USingletonLibrary::GetLevelMap())  // The Level Map is valid
		&& IS_TRANSIENT(this) == false)				  // Component is not transient
	{
		USingletonLibrary::GetLevelMap()->CharactersOnMap.Remove(this);
		USingletonLibrary::PrintToLog(this, "Destroyed", "Removed from TSet");
	}

	// Call the base class version
	Super::Destroyed();
}

void AMyCharacter::SpawnBomb()
{
	if (!IsValid(USingletonLibrary::GetLevelMap())	// The Level Map is not valid
		|| Powerups_.FireN <= 0						  // Null length of explosion
		|| Powerups_.BombN <= 0						  // No more bombs
		|| USingletonLibrary::IsEditorNotPieWorld())  // Should not spawn bomb in PIE
	{
		return;
	}

	// Spawn bomb
	auto Bomb = GetWorld()->SpawnActor<ABombActor>(USingletonLibrary::FindClassByActorType(EActorTypeEnum::Bomb), GetActorTransform());

	// Update material of mesh
	if (Bomb != nullptr)
	{
		Bomb->InitializeBombProperties(Powerups_.BombN, Powerups_.FireN, CharacterID_);
	}
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SpaceEvent", IE_Pressed, this, &AMyCharacter::SpawnBomb);
}
