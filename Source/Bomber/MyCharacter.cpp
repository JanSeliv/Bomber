// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "Bomber.h"
#include "Components/SkeletalMeshComponent.h" //ACharacter::GetMesh();

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize mapComponent
	mapComponent = CreateDefaultSubobject<UMapComponent>("Map Component");

	// Set skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skeletalMeshFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Meshes/IggyScorch"));
	if (skeletalMeshFinder.Succeeded() && ISVALID(GetMesh())) // Check to make sure the default skeletal mesh for character was actually found
	{
		GetMesh()->SetSkeletalMesh(skeletalMeshFinder.Object); // Set default skeletal mesh for character
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
	if (ISVALID(mapComponent) == false || ISTRANSIENT(this) == true)
	{
		return;
	}
	mapComponent->UpdateSelfOnMap();

	// Raise up character over cell
	if (ISVALID(GetRootComponent()) == true)
	{
		const float ACTOR_HEIGHT = GetRootComponent()->Bounds.BoxExtent.Z;
		GetRootComponent()->AddRelativeLocation(FVector(0, 0, ACTOR_HEIGHT));
	}
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
