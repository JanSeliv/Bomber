// Copyright 2020 Yevhenii Selivanov.

#include "MyGameModeBase.h"
//---
#include "MyCameraActor.h"
#include "MyHUD.h"
#include "MyPlayerController.h"
//---
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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
	UWorld* const World = GetWorld();
	if (UGameplayStatics::GetCurrentLevelName(World) == "BomberLevel")
	{
		World->SpawnActor(CameraActorClass);
	}
}
