// Copyright 2020 Yevhenii Selivanov

#include "UI/Carousel.h"
//---
#include "Bomber.h"
#include "Globals/SingletonLibrary.h"

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

#if WITH_EDITOR	 // [Editor]Destroyed();
//  Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending.
void ACarousel::Destroyed()
{
	if (USingletonLibrary::IsEditor()
	    && !IS_TRANSIENT(this))
	{
		// Remove bound delegate
		USingletonLibrary::GOnAnyDataAssetChanged.RemoveAll(this);
	}

	Super::Destroyed();
}
#endif	// WITH_EDITOR [Editor]Destroyed();
