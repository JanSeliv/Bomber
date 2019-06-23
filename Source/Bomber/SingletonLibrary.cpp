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
	if (IS_VALID(GEngine) == false)
		return nullptr;
	USingletonLibrary* singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);

	if (IS_VALID(singleton) == false)
		return nullptr;
	return singleton;
}

AGeneratedMap* const USingletonLibrary::GetLevelMap(UObject* WorldContextObject)
{
	UWorld* const world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (world == nullptr			   // World context is null
		|| GetSingleton() == nullptr)  // Singleton is null
	{
		return nullptr;
	}

// Find editor level map
#if WITH_EDITOR
	if (world->HasBegunPlay() == false					  // for editor only
		&& IS_VALID(GetSingleton()->levelMap_) == false)  // current map is not valid
	{
		TArray<AActor*> levelMapArray;
		UGameplayStatics::GetAllActorsOfClass(world, AGeneratedMap::StaticClass(), levelMapArray);
		if (levelMapArray.Num() > 0)
		{
			GetSingleton()->levelMap_ = Cast<AGeneratedMap>(levelMapArray[0]);
			UE_LOG_STR("SingletonLibrary:GetLevelMap: %s UPDATED", levelMapArray[0])
		}
	}
#endif

	return GetSingleton()->levelMap_;
}