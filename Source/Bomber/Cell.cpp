// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "Bomber.h"
#include "GameFramework/Actor.h"
#include "GeneratedMap.h"

FCell::FCell(const AActor* actor)
{
	if (!ISVALID(actor) || !ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT(actor))
		return;
	if (USingletonLibrary::GetLevelMap()->GeneratedMap_.Num() == 0)
		return;

	this->location = USingletonLibrary::GetLevelMap()->GetNearestCell(actor).location;
}
