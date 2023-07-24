// Copyright (c) Yevhenii Selivanov

#include "UI/MainMenu/CharacterSelectionSpot.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterSelectionSpot)

// Sets default values
ACharacterSelectionSpot::ACharacterSelectionSpot()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
}
