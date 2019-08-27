// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "GameFramework/Actor.h"

#include "GeneratedMap.h"
#include "SingletonLibrary.h"

const FCell FCell::ZeroCell = FCell();

FCell::FCell(const AActor* Actor)
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (LevelMap == nullptr  // The Level Map is valid and is not transient
		&& !ensureMsgf(IS_VALID(Actor), TEXT("FCell:: Actor is not valid")))
	{
		return;
	}

	USingletonLibrary::PrintToLog(Actor, "FCell()", "Start finding the cell");
	this->Location = USingletonLibrary::GetSingleton()->MakeCell(Actor).Location;  // BP implementation

	// ...
}

FCell::FCell(FVector Vector)
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	Location.X = FMath::RoundToFloat((Vector.X));
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(LevelMap ? LevelMap->GetActorLocation().Z : Vector.Z);
}