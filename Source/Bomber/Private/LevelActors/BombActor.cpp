// Copyright 2019 Yevhenii Selivanov.

#include "LevelActors/BombActor.h"

#include "Bomber.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ABombActor::ABombActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize bomb mesh component
	BombMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMeshComponent"));
	BombMeshComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BombMeshFinder(TEXT("/Game/Bomber/Assets/Meshes/BombMesh"));
	if (BombMeshFinder.Succeeded())
	{
		BombMeshComponent->SetStaticMesh(BombMeshFinder.Object);
	}

	// Initialize explosion particle component
	ExplosionParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("ExplosionParticle"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleFinder(TEXT("/Game/VFX_Toolkit_V1/ParticleSystems/356Days/Par_CrescentBoom2_OLD"));
	if (ParticleFinder.Succeeded())
	{
		ExplosionParticle = ParticleFinder.Object;
	}

	// Find bomb materials
	static TArray<ConstructorHelpers::FObjectFinder<UMaterialInterface>> MaterialsFinderArray{
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Yellow"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Blue"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Silver"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Pink")};
	for (int32 i = 0; i < MaterialsFinderArray.Num(); ++i)
	{
		if (MaterialsFinderArray[i].Succeeded())
		{
			BombMaterials.Emplace(MaterialsFinderArray[i].Object);
		}
	}

	// Initialize the Bomb Collision Component to prevent players from moving through the bomb after they moved away
	BombCollisionComponent = CreateDefaultSubobject<UBoxComponent>("BombCollisionComponent");
	BombCollisionComponent->SetupAttachment(RootComponent);
	BombCollisionComponent->SetBoxExtent(FVector(100.f));
	BombCollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void ABombActor::InitializeBombProperties(
	int32& RefBombsN,
	const int32& FireN,
	const int32& CharacterID)
{
	if (!IsValid(USingletonLibrary::GetLevelMap())  // // The Level Map is not valid
		|| IsValid(MapComponent) == false			// The Map Component is not valid
		|| FireN < 0)								// Negative length of the explosion
	{
		return;
	}

	PlayerBombsN_ = &RefBombsN;
	if (PlayerBombsN_ != nullptr)
	{
		(*PlayerBombsN_)--;
	}

	// Set material
	if (IsValid(BombMeshComponent)  // Mesh of the bomb is not valid
		&& CharacterID != -1)		// is not debug character
	{
		const int32 BombMaterialNo = FMath::Abs(CharacterID) % BombMaterials.Num();
		BombMeshComponent->SetMaterial(0, BombMaterials[BombMaterialNo]);
	}

	// Update explosion information
	USingletonLibrary::GetLevelMap()->GetSidesCells(ExplosionCells_, MapComponent->GetCell(), EPathType::Explosion, FireN);

	//#if WITH_EDITOR  // [Editor]
	//	if (MapComponent->bShouldShowRenders)
	//	{
	//		USingletonLibrary::PrintToLog(this, "[Editor]InitializeBombProperties", "-> \t AddDebugTextRenders");
	//		USingletonLibrary::AddDebugTextRenders(this, ExplosionCells_, FLinearColor::Red);
	//	}
	//#endif
}

// Called when an instance of this class is placed (in editor) or spawned.
void ABombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)			// This actor is transient
		|| !IsValid(MapComponent))  // Is not valid for map construction
	{
		return;
	}

	// Construct the actor's map component
	MapComponent->OnMapComponentConstruction();

#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld())  // [IsEditorNotPieWorld]
	{
		USingletonLibrary::PrintToLog(this, "[IsEditorNotPieWorld]OnConstruction", "-> \t InitializeBombProperties");
		InitializeBombProperties(*PlayerBombsN_, ExplosionLength_, -1);

		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
	}
#endif  //WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABombActor::OnBombDestroyed);

	// Binding to the event, that triggered when character end to overlaps the ItemCollisionComponent component
	BombCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ABombActor::OnBombEndOverlap);

	// Destroy itself after N seconds
	SetLifeSpan(LifeSpan_);
}

// Calls destroying request of all actors by cells in explosion cells array.
void ABombActor::OnBombDestroyed(AActor* DestroyedActor)
{
	UWorld* const World = GetWorld();
	if (World == nullptr								// World is null
		|| !IsValid(USingletonLibrary::GetLevelMap()))  // The Level Map is not valid
	{
		return;
	}

	// Return to the character +1 of bombs
	if (PlayerBombsN_ != nullptr)
	{
		(*PlayerBombsN_)++;
	}
	// Spawn emitters
	for (const FCell& Cell : ExplosionCells_)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionParticle, FTransform(GetActorRotation(), Cell.Location, GetActorScale3D()));
	}

	// Destroy all actors from array of cells
	USingletonLibrary::GetLevelMap()->DestroyActorsFromMap(ExplosionCells_);
}

// Sets the collision preset to block all dynamics.
void ABombActor::OnBombEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this)  // Self triggering
	{
		return;
	}
	//Sets the collision preset to block all dynamics
	TArray<AActor*> OverlappingActors;
	BombCollisionComponent->GetOverlappingActors(OverlappingActors, USingletonLibrary::GetClassByActorType(EActorType::Player));
	if (OverlappingActors.Num() == 0)  // There are no more characters on the bomb
	{
		BombCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	}
}
