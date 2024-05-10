// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"
//---
#include "NMMUtils.h"
#include "Components/MouseActivityComponent.h"
#include "Components/MyCameraComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"
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

// Assigns existing Save Game Data to this component 
void UNMMPlayerControllerComponent::SetSaveGameData(class USaveGame* NewSaveGameData)
{
	UNMMSaveGameData* InSaveGameData = Cast<UNMMSaveGameData>(NewSaveGameData);
	if (!InSaveGameData
		|| InSaveGameData == SaveGameDataInternal)
	{
		return;
	}

	SaveGameDataInternal = InSaveGameData;
}

// Enables or disables the input context during Cinematic Main Menu State
void UNMMPlayerControllerComponent::SetCinematicInputContextEnabled(bool bEnable)
{
	AMyPlayerController& MyPC = GetPlayerControllerChecked();

	if (bEnable)
	{
		// Disable all other first
		MyPC.SetAllInputContextsEnabled(false, ECurrentGameState::Max);
	}

	// Enable Cinematic inputs
	MyPC.SetInputContextEnabled(bEnable, UNMMDataAsset::Get().GetInputContext(ENMMState::Cinematic));
}

// Enables or disables Cinematic mouse settings from Player Input data asset
void UNMMPlayerControllerComponent::SetCinematicMouseVisibilityEnabled(bool bEnabled)
{
	static const FName CinematicStateName = GET_ENUMERATOR_NAME_CHECKED(ENMMState, Cinematic);
	UMouseActivityComponent& MouseActivityComponent = GetPlayerControllerChecked().GetMouseActivityComponentChecked();
	MouseActivityComponent.SetMouseVisibilitySettingsEnabledCustom(bEnabled, CinematicStateName);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the owning Actor begins play or when the component is created if the Actor has already begun play
void UNMMPlayerControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen Main Menu states
	UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();
	BaseSubsystem.OnMainMenuStateChanged.AddUniqueDynamic(this, &ThisClass::OnNewMainMenuStateChanged);
	if (BaseSubsystem.GetCurrentMenuState() != ENMMState::None)
	{
		// State is already set, apply it
		OnNewMainMenuStateChanged(BaseSubsystem.GetCurrentMenuState());
	}

	// Load save game data of the Main Menu
	FAsyncLoadGameFromSlotDelegate AsyncLoadGameFromSlotDelegate;
	AsyncLoadGameFromSlotDelegate.BindUObject(this, &ThisClass::OnAsyncLoadGameFromSlotCompleted);
	UGameplayStatics::AsyncLoadGameFromSlot(UNMMSaveGameData::GetSaveSlotName(), UNMMSaveGameData::GetSaveSlotIndex(), AsyncLoadGameFromSlotDelegate);

	// Add Menu context as auto managed by Game State, so it will be enabled everytime the game is in the Menu state
	GetPlayerControllerChecked().SetupInputContexts({UNMMDataAsset::Get().GetInputContext(ENMMState::Idle)});

	const UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	if (SpotsSubsystem.IsAnyMainMenuSpotReady())
	{
		GetPlayerControllerChecked().SetMenuState();
	}
	else
	{
		// Listen to set Menu game state once first spot is ready
		UNMMSpotsSubsystem::Get().OnMainMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnMainMenuSpotReady);
	}

	// Disable auto camera possess by default, so it can be controlled by the spot
	UMyCameraComponent* LevelCamera = UMyBlueprintFunctionLibrary::GetLevelCamera();
	if (ensureMsgf(LevelCamera, TEXT("ASSERT: [%i] %s:\n'EXPR' is not valid, can't disable Auto Camera Possess!"), __LINE__, *FString(__FUNCTION__)))
	{
		LevelCamera->SetAutoPossessCameraEnabled(false);
	}
}

// Clears all transient data created by this component
void UNMMPlayerControllerComponent::OnUnregister()
{
	// Remove all input contexts managed by Controller
	if (const UNMMDataAsset* DataAsset = UNMMUtils::GetDataAsset(this))
	{
		TArray<const UMyInputMappingContext*> MenuInputContexts;
		DataAsset->GetAllInputContexts(/*out*/MenuInputContexts);
		GetPlayerControllerChecked().RemoveInputContexts(MenuInputContexts);
	}

	// Kill current save game object
	if (SaveGameDataInternal)
	{
		SaveGameDataInternal->ConditionalBeginDestroy();
		SaveGameDataInternal = nullptr;
	}

	Super::OnUnregister();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called wen the Main Menu state was changed
void UNMMPlayerControllerComponent::OnNewMainMenuStateChanged_Implementation(ENMMState NewState)
{
	AMyPlayerController& MyPC = GetPlayerControllerChecked();

	switch (NewState)
	{
	case ENMMState::Cinematic:
		{
			MyPC.SetIgnoreMoveInput(true);
			USoundsSubsystem::Get().StopCurrentBackgroundMusic();
			break;
		}
	default: break;
	}

	// Update input contexts
	SetCinematicInputContextEnabled(NewState == ENMMState::Cinematic);

	// Update mouse visibility
	SetCinematicMouseVisibilityEnabled(NewState == ENMMState::Cinematic);
}

// Is listen to set Menu game state once first spot is ready
void UNMMPlayerControllerComponent::OnMainMenuSpotReady_Implementation(UNMMSpotComponent* MainMenuSpotComponent)
{
	checkf(MainMenuSpotComponent, TEXT("ERROR: [%i] %s:\n'MainMenuSpotComponent' is null!"), __LINE__, *FString(__FUNCTION__));
	if (MainMenuSpotComponent->IsActiveSpot())
	{
		GetPlayerControllerChecked().SetMenuState();

		UNMMSpotsSubsystem::Get().OnMainMenuSpotReady.RemoveAll(this);
	}
}

// Is called from AsyncLoadGameFromSlot once Save Game is loaded, or null if it failed to load
void UNMMPlayerControllerComponent::OnAsyncLoadGameFromSlotCompleted_Implementation(const FString& SlotName, int32 UserIndex, USaveGame* SaveGame)
{
	UNMMSaveGameData* InSaveGameData = Cast<UNMMSaveGameData>(SaveGame);
	if (!InSaveGameData)
	{
		// There is no save game, or it is corrupted, create a new one
		InSaveGameData = NewObject<UNMMSaveGameData>(this);
		InSaveGameData->SaveDataAsync();
		// Fallback to cache it
	}

	SetSaveGameData(SaveGame);
}
