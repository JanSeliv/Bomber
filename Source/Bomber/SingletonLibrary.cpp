// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonLibrary.h"

#include "Bomber.h"
#include "Engine.h"
#include "GeneratedMap.h"
#include "Kismet/GameplayStatics.h"

USingletonLibrary::USingletonLibrary()
{
}

USingletonLibrary* const USingletonLibrary::GetSingleton()
{
	if (GEngine == nullptr)
	{
		return nullptr;
	}

	return Cast<USingletonLibrary>(GEngine->GameSingleton);
}

AGeneratedMap* const USingletonLibrary::GetLevelMap(UObject* WorldContextObject)
{
	if (GEngine == nullptr			   // Global engine pointer is null
		|| GetSingleton() == nullptr)  // Singleton is null
	{
		return nullptr;
	}

// Find editor level map
#if WITH_EDITOR
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (World != nullptr &&								  // World is null
		World->HasBegunPlay() == false					  // for editor only
		&& IS_VALID(GetSingleton()->LevelMap_) == false)  // current map is not valid
	{
		TArray<AActor*> LevelMapArray;
		UGameplayStatics::GetAllActorsOfClass(World, AGeneratedMap::StaticClass(), LevelMapArray);
		if (LevelMapArray.Num() > 0)
		{
			GetSingleton()->LevelMap_ = Cast<AGeneratedMap>(LevelMapArray[0]);
			UE_LOG_STR("SingletonLibrary:GetLevelMap: %s UPDATED", LevelMapArray[0]);
		}
		else
		{
			return nullptr;
		}
	}
#endif

	return GetSingleton()->LevelMap_;
}