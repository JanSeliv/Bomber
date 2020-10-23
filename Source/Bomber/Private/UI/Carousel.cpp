// Copyright 2020 Yevhenii Selivanov

#include "Carousel.h"
//---
#include "Bomber.h"
#include "SingletonLibrary.h"

// Sets default values
ACarousel::ACarousel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called every frame
void ACarousel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when an instance of this class is placed (in editor) or spawned
void ACarousel::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR // [GEditor]
	if (GEditor
        && !IS_TRANSIENT(this)
        && !USingletonLibrary::GOnAnyDataAssetChanged.IsBoundToObject(this))
	{
		USingletonLibrary::GOnAnyDataAssetChanged.AddUObject(this, &ThisClass::RerunConstructionScripts);
	}
#endif //WITH_EDITOR [GEditor]
}

// Called when the game starts or when spawned
void ACarousel::BeginPlay()
{
	Super::BeginPlay();

}
