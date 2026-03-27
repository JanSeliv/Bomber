// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMCameraSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Components/BmrCameraComponent.h"
#include "Components/BmrMouseActivityComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "DalSubsystem.h"
#include "DataAssets/BmrInputMappingContext.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "MyUtilsLibraries/SaveUtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "Subsystems/BmrSoundsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerState.h"

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
		// Disable all other first (ParentTag matches all game state contexts)
		MyPC.SetAllInputContextsEnabled(false, FBmrGameStateTag::ParentTag);
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
	    && !InputContext->GetActiveForStates().IsEmpty())
	{
		PC.SetupInputContexts({InputContext});
	}
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

	UDalSubsystem::Get().ListenForDataAsset<UNMMDataAsset>(this, &ThisClass::OnDataAssetLoaded);
}

// Clears all transient data created by this component
void UNMMPlayerControllerComponent::OnUnregister()
{
	// Notify that Main Menu is being unloaded before any cleanup
	ABmrPlayerController* MyPC = GetPlayerController();
	const ABmrPawn* LocalPawn = MyPC ? MyPC->GetPawn<ABmrPawn>() : nullptr;
	if (LocalPawn)
	{
		FGameplayEventData EventData;
		EventData.EventTag = NmmGameplayTags::Event::MenuUnloaded;
		EventData.Instigator = LocalPawn;
		UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);

		if (!MyPC->HasAuthority())
		{
			MyPC->ServerBroadcastMessage(EventData);
		}
	}

	if (const UNMMDataAsset* DataAsset = UNMMUtils::GetDataAsset())
	{
		// Unregister all input actions and input contexts
		TArray<const UBmrInputMappingContext*> MenuInputContexts;
		DataAsset->GetAllInputContexts(/*out*/ MenuInputContexts);

		for (const UBmrInputMappingContext* InputContextIt : MenuInputContexts)
		{
			UInputUtilsLibrary::UnbindInputActionsInContext(MyPC, InputContextIt);
		}
		MyPC->RemoveInputContexts(MenuInputContexts);
	}

	// Cleanup all sounds
	const UNMMDataAsset* SoundDataAsset = UNMMUtils::GetDataAsset();
	UBmrSoundsSubsystem* SoundSubsystem = UBmrSoundsSubsystem::GetSoundsSubsystem();
	if (SoundSubsystem
	    && SoundDataAsset)
	{
		TArray<USoundBase*> AllMainMenuMusic;
		SoundDataAsset->GetAllMainMenuMusic(/*out*/ AllMainMenuMusic);
		for (USoundBase* MainMenuMusic : AllMainMenuMusic)
		{
			SoundSubsystem->DestroySingleSound2D(MainMenuMusic);
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

// Called when the NMM data asset is loaded and available
void UNMMPlayerControllerComponent::OnDataAssetLoaded_Implementation(const UNMMDataAsset* DataAsset)
{
	constexpr int32 FirstPlayerId = 0;
	BIND_ON_PAWN_READY_ID(this, ThisClass::OnFirstPawnReady, FirstPlayerId);

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

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

// Called when the first player character is spawned, possessed, and replicated
void UNMMPlayerControllerComponent::OnFirstPawnReady_Implementation(const FGameplayEventData& Payload)
{
	// Once player is initialized, listen for for menu spots (dont attempt earlier, so we dont attempt to set menu state before player is ready)
	UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	if (SpotsSubsystem.IsActiveMenuSpotReady())
	{
		OnActiveMenuSpotReady(SpotsSubsystem.GetCurrentSpot());
	}
	else
	{
		SpotsSubsystem.OnActiveMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnActiveMenuSpotReady);
	}
}

// Listen to react when entered the Menu state
void UNMMPlayerControllerComponent::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::Menu))
	{
		// Entered the Main Menu
		PlayMainMenuMusic();
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		// Left the Main Menu
		StopMainMenuMusic();
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
	// Notify that Main Menu camera spot is ready
	FGameplayEventData EventData;
	EventData.EventTag = NmmGameplayTags::Event::ActiveSpotReady;
	EventData.Instigator = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	EventData.OptionalObject = MainMenuSpotComponent;
	UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);

	// This event might be triggered only by local client, which only one enters to menu, so broadcast server that this client has menu system initialized and ready
	ABmrPlayerController& MyPC = GetPlayerControllerChecked();
	if (!MyPC.HasAuthority())
	{
		MyPC.ServerBroadcastMessage(EventData);
	}

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
