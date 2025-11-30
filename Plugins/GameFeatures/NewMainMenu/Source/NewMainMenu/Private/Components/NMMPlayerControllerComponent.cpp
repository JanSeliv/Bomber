// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "NMMUtils.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Components/BmrMouseActivityComponent.h"
#include "Components/BmrCameraComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrInputMappingContext.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/SaveUtilsLibrary.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "Subsystems/BmrSoundsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/AudioComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMPlayerControllerComponent)

// Sets default values for this component's properties
UNMMPlayerControllerComponent::UNMMPlayerControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns Player Controller of this component
ABmrPlayerController* UNMMPlayerControllerComponent::GetPlayerController() const
{
	return Cast<ABmrPlayerController>(GetOwner());
}

ABmrPlayerController& UNMMPlayerControllerComponent::GetPlayerControllerChecked() const
{
	ABmrPlayerController* MyPlayerController = GetPlayerController();
	checkf(MyPlayerController, TEXT("%s: 'MyPlayerController' is null"), *FString(__FUNCTION__));
	return *MyPlayerController;
}

/*********************************************************************************************
 * Main methods
 ********************************************************************************************* */

// Assigns existing Save Game Data to this component
void UNMMPlayerControllerComponent::SetSaveGameData(class USaveGame* NewSaveGameData)
{
	UNMMSaveGameData* InSaveGameData = Cast<UNMMSaveGameData>(NewSaveGameData);
	if (!InSaveGameData
	    || InSaveGameData == SaveGameData)
	{
		return;
	}

	SaveGameData = InSaveGameData;
}

// Enables or disables the input context during Cinematic Main Menu State
void UNMMPlayerControllerComponent::SetCinematicInputContextEnabled(bool bEnable)
{
	ABmrPlayerController& MyPC = GetPlayerControllerChecked();

	if (bEnable)
	{
		// Disable all other first
		MyPC.SetAllInputContextsEnabled(false, EBmrCurrentGameState::Max);
	}

	// Enable Cinematic inputs
	MyPC.SetInputContextEnabled(bEnable, UNMMDataAsset::Get().GetInputContext(ENMMState::Cinematic));
}

// Enables or disables Cinematic mouse settings from Player Input data asset
void UNMMPlayerControllerComponent::SetCinematicMouseVisibilityEnabled(bool bEnabled)
{
	static const FName CinematicStateName = GET_ENUMERATOR_NAME_CHECKED(ENMMState, Cinematic);
	UBmrMouseActivityComponent& MouseActivityComponent = GetPlayerControllerChecked().GetMouseActivityComponentChecked();
	MouseActivityComponent.SetMouseVisibilitySettingsEnabledCustom(bEnabled, CinematicStateName);
}

// Enables or disables the input context according to new menu state
void UNMMPlayerControllerComponent::SetManagedInputContextsEnabled(ENMMState NewState)
{
	if (UNMMUtils::GetMainMenuWidget() == nullptr)
	{
		// Widgets are not initialized yet, it will be handled later
		return;
	}

	ABmrPlayerController& PC = GetPlayerControllerChecked();

	// Remove all previous input contexts managed by Controller
	TArray<const UBmrInputMappingContext*> OutInputContexts;
	UNMMDataAsset::Get().GetAllInputContexts(/*out*/ OutInputContexts);
	PC.RemoveInputContexts(OutInputContexts);

	// Add Menu context as auto managed by Game State, so it will be enabled everytime the game is in the Menu state
	const UBmrInputMappingContext* InputContext = UNMMDataAsset::Get().GetInputContext(NewState);
	if (InputContext
	    && InputContext->GetChosenGameStatesBitmask() > 0)
	{
		PC.SetupInputContexts({InputContext});
	}
}

// Tries to set the Menu game state on initializing the Main Menu system
void UNMMPlayerControllerComponent::TrySetMenuState()
{
	if (ABmrGameState::GetCurrentGameState() != EBmrCurrentGameState::None)
	{
		// No need to initialize the Menu state, game state was already set by other systems
		return;
	}

	GetPlayerControllerChecked().SetMenuState();
}

/*********************************************************************************************
 * Sounds
 ********************************************************************************************* */

// Trigger the background music to be played in the Main Menu
void UNMMPlayerControllerComponent::PlayMainMenuMusic()
{
	const EBmrLevelType LevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	USoundBase* MainMenuMusic = UNMMDataAsset::Get().GetMainMenuMusic(LevelType);

	if (!MainMenuMusic)
	{
		// Background music is not found for current state or level, disable current
		StopMainMenuMusic();
		return;
	}

	UBmrSoundsSubsystem::Get().PlaySingleSound2D(MainMenuMusic);
}

// Stops currently played Main Menu background music
void UNMMPlayerControllerComponent::StopMainMenuMusic()
{
	const EBmrLevelType LevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	if (USoundBase* MainMenuMusic = UNMMDataAsset::Get().GetMainMenuMusic(LevelType))
	{
		UBmrSoundsSubsystem::Get().StopSingleSound2D(MainMenuMusic);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the owning Actor begins play or when the component is created if the Actor has already begun play
void UNMMPlayerControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// Listen to set Menu game state once active spot is ready
	UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	if (SpotsSubsystem.IsActiveMenuSpotReady())
	{
		OnActiveMenuSpotReady(SpotsSubsystem.GetCurrentSpot());
	}
	else
	{
		SpotsSubsystem.OnActiveMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnActiveMenuSpotReady);
	}

	BIND_ON_MENU_STATE_CHANGED(this, ThisClass::OnNewMainMenuStateChanged);

	// Load save game data of the Main Menu
	FAsyncLoadGameFromSlot AsyncLoadGameFromSlotDelegate;
	AsyncLoadGameFromSlotDelegate.BindUObject(this, &ThisClass::OnAsyncLoadGameFromSlotCompleted);
	USaveUtilsLibrary::AsyncLoadGameFromSlot(this, UNMMSaveGameData::GetSaveSlotName(), UNMMSaveGameData::GetSaveSlotIndex(), AsyncLoadGameFromSlotDelegate);

	// Disable auto camera possess by default, so it can be controlled by the spot
	UBmrCameraComponent* LevelCamera = UBmrBlueprintFunctionLibrary::GetLevelCamera();
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
		TArray<const UBmrInputMappingContext*> MenuInputContexts;
		DataAsset->GetAllInputContexts(/*out*/ MenuInputContexts);
		GetPlayerControllerChecked().RemoveInputContexts(MenuInputContexts);

		// Cleanup all sounds
		if (UBmrSoundsSubsystem* SoundSubsystem = UBmrSoundsSubsystem::GetSoundsSubsystem())
		{
			TArray<class USoundBase*> AllMainMenuMusic;
			UNMMDataAsset::Get().GetAllMainMenuMusic(/*out*/ AllMainMenuMusic);
			for (USoundBase* MainMenuMusic : AllMainMenuMusic)
			{
				SoundSubsystem->DestroySingleSound2D(MainMenuMusic);
			}
		}
	}

	// Kill current save game object
	if (SaveGameData)
	{
		SaveGameData->ConditionalBeginDestroy();
		SaveGameData = nullptr;
	}

	Super::OnUnregister();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Listen to react when entered the Menu state
void UNMMPlayerControllerComponent::OnGameStateChanged(EBmrCurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECGS::Menu: // Entered the Main Menu
			PlayMainMenuMusic();
			break;
		case ECGS::GameStarting: // Left the Main Menu
			StopMainMenuMusic();
			break;
		default:
			break;
	}
}

// Called wen the Main Menu state was changed
void UNMMPlayerControllerComponent::OnNewMainMenuStateChanged_Implementation(ENMMState NewState, ENMMState PreviousState)
{
	ABmrPlayerController& MyPC = GetPlayerControllerChecked();

	switch (NewState)
	{
		case ENMMState::Cinematic:
			MyPC.SetIgnoreMoveInput(true);
			StopMainMenuMusic();
			break;
		default:
			break;
	}

	// Update input contexts
	SetManagedInputContextsEnabled(NewState);

	// Update input contexts
	SetCinematicInputContextEnabled(NewState == ENMMState::Cinematic);

	// Update mouse visibility
	SetCinematicMouseVisibilityEnabled(NewState == ENMMState::Cinematic);
}

// Is listen to set Menu game state once first spot is ready
void UNMMPlayerControllerComponent::OnActiveMenuSpotReady_Implementation(UNMMSpotComponent* MainMenuSpotComponent)
{
	TrySetMenuState();

	UNMMCameraSubsystem::Get().PossessCamera(ENMMState::Idle);

	UNMMSpotsSubsystem::Get().OnActiveMenuSpotReady.RemoveAll(this);
}

// Is called from AsyncLoadGameFromSlot once Save Game is loaded, or null if it failed to load
void UNMMPlayerControllerComponent::OnAsyncLoadGameFromSlotCompleted_Implementation(USaveGame* SaveGame)
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
