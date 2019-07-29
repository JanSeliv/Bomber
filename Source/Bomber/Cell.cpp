// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "GameFramework/Actor.h"
#include "SingletonLibrary.h"

const FCell FCell::ZeroCell = FCell();

FCell::FCell(const AActor* Actor)
{
	check(Actor);
	if (USingletonLibrary::GetSingleton() == nullptr)  // Singleton is null
	{
		return;
	}
	USingletonLibrary::PrintToLog(Actor, "FCell()", "Start finding the cell");
	this->Location = USingletonLibrary::GetSingleton()->MakeCell(Actor).Location;
}
