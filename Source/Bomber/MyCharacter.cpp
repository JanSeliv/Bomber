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
	PrimaryActorTick.bCanEverTick = true;

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));

	// Set skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshFinder(TEXT("/Game/ParagonIggyScorch/Characters/Heroes/IggyScorch/Meshes/IggyScorch"));
	if (SkeletalMeshFinder.Succeeded())  // Check to make sure the default skeletal mesh for character was actually found
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshFinder.Object);  // Set default skeletal mesh for character
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

	if (IS_VALID(MapComponent) == false)  // Map component is not valid
	{
		return;
	}

	if (IsChildActor() == false)  // Was dragged to PIE and it needs to update
	{
		MapComponent->UpdateSelfOnMap();
	}

	// Raise up character over cell
	const float ActorHeight = GetRootComponent()->Bounds.BoxExtent.Z;
	AddActorWorldOffset(FVector(0.f, 0.f, ActorHeight));
	SetActorRotation(FRotator(0, -90, 0));

	UE_LOG_STR("OnConstruction:LocationAndRotation: %s", this);

// Binding to update renders of render AI on creating\destroying elements
#if WITH_EDITOR
	if (GetWorld()->HasBegunPlay() == false  // for editor only
		&& bShouldShowRenders == true)		 // only for AI with render statement
	{
		USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.AddDynamic(this, &AMyCharacter::UpdateAI);
		UE_LOG_STR("PIE: %s BINDING to UpdateAI", this);
	}
#endif
}

void AMyCharacter::SpawnBomb()
{
	if (!IS_VALID(USingletonLibrary::GetLevelMap(GetWorld()))  // level map is not valid
		|| Powerups_.FireN <= 0								   // Null length of explosion
		|| Powerups_.BombN <= 0								   // No more bombs
		|| HasActorBegunPlay() == false						   // Should not spawn bomb in PIE
		|| IS_VALID(MapComponent) == false)					   // Map component is not valid
	{
		return;
	}

	// Spawn bomb
	ABomb* const Bomb = Cast<ABomb>(USingletonLibrary::GetLevelMap(GetWorld())->AddActorOnMap(FCell(this), EActorTypeEnum::Bomb));

	// Update material of mesh
	if (Bomb != nullptr)
	{
		Bomb->InitializeBombProperties(&Powerups_.BombN, Powerups_.FireN, CharacterID_);
		Powerups_.BombN--;
	}
}

void AMyCharacter::UpdateAI_Implementation()
{
// Check who answered the call
#if WITH_EDITOR
	if (GetWorld()->HasBegunPlay() == false)  // for editor only
	{
		UE_LOG_STR("PIE:UpdateAI: %s answered", this);
	}
#endif
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SpaceEvent", IE_Pressed, this, &AMyCharacter::SpawnBomb);
}
