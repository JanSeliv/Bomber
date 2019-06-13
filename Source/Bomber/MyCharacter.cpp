// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "Bomber.h"
#include "Components/SkeletalMeshComponent.h" //ACharacter::GetMesh();

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));

	// Initialize mapComponent
	mapComponent = CreateDefaultSubobject<UMapComponent>("Map Component");
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
		return;

	mapComponent->UpdateSelfOnMap();
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
