// Copyright (c) Yevhenii Selivanov

#include "UI/MainMenu/CharacterSelectionSpot.h"
//---
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "UI/MyHUD.h"
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

// Overridable native event for when play begins for this actor.
void ACharacterSelectionSpot::BeginPlay()
{
	Super::BeginPlay();

	if (AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD())
	{
		MyHUD->AddCharacterSelectionSpot(this);
	}
}
