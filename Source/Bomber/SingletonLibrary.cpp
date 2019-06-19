// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonLibrary.h"

#include "Bomber.h"
#include "Engine/Engine.h"
#include "GeneratedMap.h"
#include "Kismet/GameplayStatics.h"

USingletonLibrary::USingletonLibrary()
{
}

USingletonLibrary* const USingletonLibrary::GetSingleton()
{
	if (IsValid(GEngine) == false)
		return nullptr;
	USingletonLibrary* singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);

	if (IsValid(singleton) == false)
		return nullptr;
	return singleton;
}

AGeneratedMap* const USingletonLibrary::GetLevelMap(UObject* WorldContextObject)
{
	UWorld* const world = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (world == nullptr					  // World context is null
		|| IsValid(GetSingleton()) == false)  // Singleton is not valid
	{
		return nullptr;
	}

	// Find editor level map
#if WITH_EDITOR
	if (ISVALID(GetSingleton()->levelMap_) == false)
	{
		TArray<AActor*> levelMapArray;
		UGameplayStatics::GetAllActorsOfClass(world, AGeneratedMap::StaticClass(), levelMapArray);
		if (levelMapArray.Num() > 0)
		{
			GetSingleton()->levelMap_ = Cast<AGeneratedMap>(levelMapArray[0]);
		}
	}
#endif

	return GetSingleton()->levelMap_;
}