// Fill out your copyright notice in the Description page of Project Settings.


#include "SingletonLibrary.h"
#include "Bomber.h"

USingletonLibrary::USingletonLibrary()
{
	TArray<TCHAR*> pathes{
	TEXT("/Game/Bomber/Blueprints/BPCameraActor"),	// EActorTypeEnum::None
	TEXT("/Game/Bomber/Blueprints/BpBomb"),			// EActorTypeEnum::Bomb
	TEXT("/Game/Bomber/Blueprints/BpItem"),			// EActorTypeEnum::Item
	TEXT("/Game/Bomber/Assets/Wall"),				// EActorTypeEnum::Wall
	TEXT("/Game/Bomber/Assets/floor"),				// EActorTypeEnum::Floor
	TEXT("/Game/Bomber/Assets/Box"),				// EActorTypeEnum::Box
	TEXT("/Game/Bomber/Blueprints/BpPlayer"),		// EActorTypeEnum::Player
	};
	for (auto path : pathes)
	{
		ConstructorHelpers::FClassFinder<AActor> classFinder(path);
		bpClasses.Add(classFinder.Class);
	}
}

USingletonLibrary* const USingletonLibrary::GetSingleton()
{
	if (IsValid(GEngine) == false) return nullptr;
	USingletonLibrary* singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);

	if (IsValid(singleton) == false) return nullptr;
	return singleton;
}

FCell USingletonLibrary::MakeCell(const FVector& cellLocation, bool bShoudForce = false)
{
	FCell cell;
	cell.location = cellLocation;
	if (bShoudForce == true) return cell;

	//Find nearest
	/* ... */
	return cell;
}

