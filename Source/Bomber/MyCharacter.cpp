// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"

#include "Bomb.h"
#include "Bomber.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"  //ACharacter::GetMesh();
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize skeletal mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));

	// Set the skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Meshes/IggyScorch"));
	if (SkeletalMeshFinder.Succeeded())  // Check to make sure the default skeletal mesh for character was actually found
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshFinder.Object);  // Set default skeletal mesh for character
	}

	// Set the animation
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimationFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/IggyScorch_AnimBP"));
	if (AnimationFinder.Succeeded())  // The animation was found
	{
		GetMesh()->AnimClass = AnimationFinder.Object->GeneratedClass;
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

	if (IS_VALID(MapComponent) == false)  // this component is not valid for owner construction
	{
		return;
	}

	// Construct the actor's map component
	MapComponent->OnMapComponentConstruction();

	// Rotate character
	SetActorRotation(FRotator(0.f, -90.f, 0.f));
	USingletonLibrary::PrintToLog(this, "OnConstruction \t New rotation:", GetActorRotation().ToString());
}

void AMyCharacter::Destroyed()
{
	UWorld* const World = GetWorld();
	if (World != nullptr										 // World is not null
		&& IS_VALID(USingletonLibrary::GetLevelMap(GetWorld()))  // The Level Map is valid
		&& IS_TRANSIENT(this) == false)							 // Component is not transient
	{
		USingletonLibrary::GetLevelMap(World)->CharactersOnMap.Remove(this);
		USingletonLibrary::PrintToLog(this, "Destroyed", "Removed from TSet");
	}

	// Call the base class version
	Super::Destroyed();
}

void AMyCharacter::SpawnBomb()
{
	if (!IS_VALID(USingletonLibrary::GetLevelMap(GetWorld()))  // The Level Map is not valid
		|| Powerups_.FireN <= 0								   // Null length of explosion
		|| Powerups_.BombN <= 0								   // No more bombs
		|| IS_PIE(GetWorld()) == true)						   // Should not spawn bomb in PIE
	{
		return;
	}

	// Spawn bomb
	ABomb* const Bomb = GetWorld()->SpawnActor<ABomb>(*USingletonLibrary::FindClassByActorType(EActorTypeEnum::Bomb), GetActorTransform());

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
