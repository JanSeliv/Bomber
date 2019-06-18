// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "Bomber.h"
#include "GameFramework/Actor.h"
#include "GeneratedMap.h"

FCell::FCell(const AActor* actor)
{
	if (ISVALID(actor) == false											// cell actor is not valid
		|| ISVALID(USingletonLibrary::GetLevelMap()) == false			// levelMap is not valid
		|| USingletonLibrary::GetLevelMap()->GeneratedMap_.Num() == 0)  // empty Grid Array
	{
		return;
	}

	this->location = USingletonLibrary::GetLevelMap()->GetNearestCell(actor).location;
}
