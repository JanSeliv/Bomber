// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"
//---
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMPlayerControllerComponent)

// Sets default values for this component's properties
UNMMPlayerControllerComponent::UNMMPlayerControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns HUD actor of this component
AMyPlayerController* UNMMPlayerControllerComponent::GetPlayerController() const
{
	return Cast<AMyPlayerController>(GetOwner());
}

AMyPlayerController& UNMMPlayerControllerComponent::GetPlayerControllerChecked() const
{
	AMyPlayerController* MyPlayerController = GetPlayerController();
	checkf(MyPlayerController, TEXT("%s: 'MyPlayerController' is null"), *FString(__FUNCTION__));
	return *MyPlayerController;
}

// Called when the owning Actor begins play or when the component is created if the Actor has already begun play
void UNMMPlayerControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<const UMyInputMappingContext*> MenuInputContexts;
	UNMMDataAsset::Get().GetAllInputContexts(/*out*/MenuInputContexts);
	GetPlayerControllerChecked().SetupInputContexts(MenuInputContexts);
}
