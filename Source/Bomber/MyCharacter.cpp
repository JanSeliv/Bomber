// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"

#include "Bomb.h"
#include "Bomber.h"
#include "Components/SkeletalMeshComponent.h"  //ACharacter::GetMesh();
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize mapComponent
	mapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));

	// Set skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skeletalMeshFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Meshes/IggyScorch"));
	if (skeletalMeshFinder.Succeeded())  // Check to make sure the default skeletal mesh for character was actually found
	{
		GetMesh()->SetSkeletalMesh(skeletalMeshFinder.Object);  // Set default skeletal mesh for character
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	}
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMyCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ISVALID(mapComponent) == false)
	{
		return;
	}

#if WITH_EDITOR
	if (bShouldShowRenders == true)
	{
		USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.AddDynamic(this, &AMyCharacter::UpdateAI);
	}
#endif

	mapComponent->UpdateSelfOnMap();
}

void AMyCharacter::SpawnBomb()
{
	if (!ISVALID(USingletonLibrary::GetLevelMap(GetWorld()))  // level map is not valid
		|| powerups_.fireN == 0								  // Null length of explosion
		|| HasActorBegunPlay() == false						  // Shouldt spawn bomb in PIE
		|| ISVALID(mapComponent) == false)					  // Map component is not valid
	{
		return;
	}

	// Spawn bomb
	ABomb* const bomb = Cast<ABomb>(USingletonLibrary::GetLevelMap(GetWorld())->AddActorOnMap(FCell(this), EActorTypeEnum::Bomb));

	// Update material of mesh
	if (bomb != nullptr)
	{
		bomb->InitializeBombProperties(&powerups_.bombN, powerups_.fireN, characterID_);
		powerups_.bombN--;
	}
}

void AMyCharacter::UpdateAI_Implementation()
{
	UE_LOG_STR("%s", *"AMyCharacter::AI updated");
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SpaceEvent", IE_Pressed, this, &AMyCharacter::SpawnBomb);
}
