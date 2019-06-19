// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "Bomber.h"
#include "GameFramework/Actor.h"
#include "GeneratedMap.h"

FCell::FCell(const AActor* actor)
{
	check(actor);
	AGeneratedMap* const levelMap = USingletonLibrary::GetLevelMap(actor->GetWorld());
	if (levelMap == nullptr)  // levelMap is null
	{
		return;
	}
	this->location = levelMap->GetNearestCell(actor).location;
}
