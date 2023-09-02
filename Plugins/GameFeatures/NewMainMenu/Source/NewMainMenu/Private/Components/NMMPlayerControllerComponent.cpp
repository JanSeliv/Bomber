// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"
//---
#include "Components/MyCameraComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "Data/NMMSubsystem.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Kismet/GameplayStatics.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMPlayerControllerComponent)

// Sets default values for this component's properties
UNMMPlayerControllerComponent::UNMMPlayerControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns Player Controller of this component
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

// Removes all saved data of the Main Menu
void UNMMPlayerControllerComponent::ResetSaveGameData()
{
	const FString& SlotName = UNMMSaveGameData::GetSaveSlotName();
	const int32 UserIndex = UNMMSaveGameData::GetSaveSlotIndex();

	// Remove the data from the disk
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	}

	// Kill current save game object
	if (IsValid(SaveGameDataInternal))
	{
		SaveGameDataInternal->ConditionalBeginDestroy();
	}

	// Create new save game object
	SaveGameDataInternal = CastChecked<UNMMSaveGameData>(UGameplayStatics::CreateSaveGameObject(UNMMSaveGameData::StaticClass()));
	SaveGameDataInternal->SaveDataAsync();
}

// Set new sound volume for Cinematics sound class
void UNMMPlayerControllerComponent::SetCinematicsVolume(double InVolume)
{
	CinematicsVolumeInternal = InVolume;

	USoundClass* CinematicsSoundClass = UNMMDataAsset::Get().GetCinematicsSoundClass();
	USoundsSubsystem::Get().SetSoundVolumeByClass(CinematicsSoundClass, InVolume);
}

// Called when the owning Actor begins play or when the component is created if the Actor has already begun play
void UNMMPlayerControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load save game data of the Main Menu
	FAsyncLoadGameFromSlotDelegate AsyncLoadGameFromSlotDelegate;
	AsyncLoadGameFromSlotDelegate.BindUObject(this, &ThisClass::OnAsyncLoadGameFromSlotCompleted);
	UGameplayStatics::AsyncLoadGameFromSlot(UNMMSaveGameData::GetSaveSlotName(), UNMMSaveGameData::GetSaveSlotIndex(), AsyncLoadGameFromSlotDelegate);

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

// Is called from AsyncLoadGameFromSlot once Save Game is loaded, or null if it failed to load
void UNMMPlayerControllerComponent::OnAsyncLoadGameFromSlotCompleted_Implementation(const FString& SlotName, int32 UserIndex, USaveGame* SaveGame)
{
	if (SaveGame)
	{
		SaveGameDataInternal = CastChecked<UNMMSaveGameData>(SaveGame);
		return;
	}

	// There is no save game or it is corrupted, create a new one
	ResetSaveGameData();
}
