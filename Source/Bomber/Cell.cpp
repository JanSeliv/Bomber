// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "GameFramework/Actor.h"
#include "SingletonLibrary.h"

FCell::FCell()
	: Location(FVector::ZeroVector)
{
}

FCell::FCell(const AActor* Actor)
{
	check(Actor);
	if (USingletonLibrary::GetSingleton() == nullptr)  // Singleton is null
	{
		return;
	}

	this->Location = USingletonLibrary::GetSingleton()->MakeCell(Actor).Location;
}
