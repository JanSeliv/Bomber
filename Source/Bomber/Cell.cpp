// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "Bomber.h"
#include "GameFramework/Actor.h"
#include "GeneratedMap.h"
#include "SingletonLibrary.h"

FCell::FCell(const AActor* Actor)
{
	check(Actor);
	AGeneratedMap* const levelMap = USingletonLibrary::GetLevelMap(Actor->GetWorld());
	if (levelMap == nullptr)  // levelMap is null
	{
		return;
	}
	this->Location = levelMap->GetNearestCell(Actor).Location;
}
