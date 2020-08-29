// Copyright 2020 Yevhenii Selivanov.

#include "MyGameModeBase.h"
//---
#include "MyCameraActor.h"
#include "MyHUD.h"
#include "MyPlayerController.h"
#include "SingletonLibrary.h"
//---
#include "Engine/World.h"

// Sets default values for this actor's properties
AMyGameModeBase::AMyGameModeBase()
{
	// Custom defaults classes
	HUDClass = AMyHUD::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	CameraActorClass = AMyCameraActor::StaticClass();
}

// Called when the game starts or when spawned
void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Camera spawning
	//@TODO Cpawn camera on changing a game state
	UWorld* World = GetWorld();
	if (World /*&& World->GetName() == USingletonLibrary::GetMainLevelName()*/) // is main level
	{
		World->SpawnActor(CameraActorClass);
	}
}
