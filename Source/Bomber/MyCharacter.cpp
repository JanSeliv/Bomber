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
	ACharacter* character = Cast<ACharacter>(mapComponent->owner);
	if (USingletonLibrary::GetLevelMap()->charactersOnMap_.Contains(character))
	{
		UE_LOG_STR("MyCharacter: %s founded in TSet", *this->GetFName().ToString());
	}

	for (auto i : USingletonLibrary::GetLevelMap()->charactersOnMap_)
	{
		FString str = (i == character ? "true" : "false");
		UE_LOG_STR("MyCharacter: i == this - %s", *str);
	}
}

void AMyCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ISVALID(mapComponent) == false) return;

	mapComponent->UpdateSelfOnMap();

}

void AMyCharacter::Destroyed()
{
	/*
	// remove character from charactersOnMap_
	if (USingletonLibrary::GetLevelMap()->charactersOnMap_.Contains(this))
	{
		USingletonLibrary::GetLevelMap()->charactersOnMap_.Remove(this);
	}
	// fix null keys
	USingletonLibrary::GetLevelMap()->charactersOnMap_.CompactStable();
	USingletonLibrary::GetLevelMap()->charactersOnMap_.Shrink();
	UE_LOG_STR("Num of characters: %s", *FString::FromInt(USingletonLibrary::GetLevelMap()->charactersOnMap_.Num()));
	*/

	Super::Destroyed();
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

