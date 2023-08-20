// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"
//---
#include "Components/MyCameraComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
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

// Set to skips previously seen cinematics automatically
void UNMMPlayerControllerComponent::SetAutoSkipCinematicsSetting(bool bEnable)
{
	bAutoSkipCinematicsSettingInternal = bEnable;
}

// Called when the owning Actor begins play or when the component is created if the Actor has already begun play
void UNMMPlayerControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Setup Main menu inputs
	TArray<const UMyInputMappingContext*> MenuInputContexts;
	UNMMDataAsset::Get().GetAllInputContexts(/*out*/MenuInputContexts);
	GetPlayerControllerChecked().SetupInputContexts(MenuInputContexts);

	// Listen to set Menu game state once first spot is ready
	UNMMSubsystem::Get().OnMainMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnMainMenuSpotReady);

	// Disable auto camera possess by default, so it can be controlled by the spot
	UMyCameraComponent* LevelCamera = UMyBlueprintFunctionLibrary::GetLevelCamera();
	if (ensureMsgf(LevelCamera, TEXT("ASSERT: [%i] %s:\n'EXPR' is not valid, can't disable Auto Camera Possess!"), __LINE__, *FString(__FUNCTION__)))
	{
		LevelCamera->SetAutoPossessCameraEnabled(false);
	}
}

// Is listen to set Menu game state once first spot is ready
void UNMMPlayerControllerComponent::OnMainMenuSpotReady_Implementation(UNMMSpotComponent* MainMenuSpotComponent)
{
	checkf(MainMenuSpotComponent, TEXT("ERROR: [%i] %s:\n'MainMenuSpotComponent' is null!"), __LINE__, *FString(__FUNCTION__));
	if (MainMenuSpotComponent->IsActiveSpot())
	{
		GetPlayerControllerChecked().SetMenuState();
	}
}
