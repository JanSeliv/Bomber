// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"
#include "Bomber.h"

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	if (IsValid(USingletonLibrary::GetSingleton()))
	{
		USingletonLibrary::GetSingleton()->levelMap = this;
	}

}


// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

}
