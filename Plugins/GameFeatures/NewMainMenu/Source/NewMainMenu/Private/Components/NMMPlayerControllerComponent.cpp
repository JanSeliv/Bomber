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

	// Listen to set Menu game state once first spot is ready
	UNMMSpotsSubsystem::Get().OnMainMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnMainMenuSpotReady);

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
			break;
		}
	default: break;
	}

	// Update input contexts
	SetCinematicInputContextEnabled(NewState == ENMMState::Cinematic);

	// Update mouse visibility
	UMouseActivityComponent* MouseActivityComponent = MyPC.GetMouseActivityComponent();
	checkf(MouseActivityComponent, TEXT("ERROR: [%i] %s:\n'MouseActivityComponent' is null!"), __LINE__, *FString(__FUNCTION__));
	const FMouseVisibilitySettings& NewMouseSettings = UNMMDataAsset::Get().GetMouseVisibilitySettings(NewState);
	MouseActivityComponent->SetMouseVisibilitySettings(NewMouseSettings);
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
