// Copyright (c) Yevhenii Selivanov

#include "Components/NMMPlayerControllerComponent.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "Data/NMMDataAsset.h"
#include "Data/NMMSaveGameData.h"
#include "Data/NmmStateTag.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"

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
#include "Subsystems/BmrSoundsSubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
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
	TArray<UBmrInputMappingContext*> CinematicContexts;
	UNMMDataAsset::Get().GetInputContexts(FNmmStateTag::Cinematic, /*out*/ CinematicContexts);
	MyPC.SetAllInputContextEnabled(bEnable, CinematicContexts);
}

// Enables or disables Cinematic mouse settings from Player Input data asset
void UNMMPlayerControllerComponent::SetCinematicMouseVisibilityEnabled(bool bEnabled)
{
	static const FName CinematicMouseCustom = TEXT("Cinematic");
	UBmrMouseActivityComponent& MouseActivityComponent = GetPlayerControllerChecked().GetMouseActivityComponentChecked();
	MouseActivityComponent.SetMouseVisibilitySettingsEnabledCustom(bEnabled, CinematicMouseCustom);
}

// Enables or disables the input context according to new menu state
void UNMMPlayerControllerComponent::SetManagedInputContextsEnabled(FNmmStateTag NewMenuState)
{
	if (UNMMUtils::GetMainMenuWidget() == nullptr)
	{
		// Widgets are not initialized yet, it will be handled later
		return;
	}

	ABmrPlayerController& PC = GetPlayerControllerChecked();

	// Remove all previous input contexts managed by Controller
	TArray<UBmrInputMappingContext*> AllContexts;
	UNMMDataAsset::Get().GetAllInputContexts(/*out*/ AllContexts);
	PC.RemoveInputContexts(AllContexts);

	// Add Menu contexts as auto managed by Game State, so they will be enabled everytime the game is in the Menu state
	TArray<UBmrInputMappingContext*> MatchingContexts;
	UNMMDataAsset::Get().GetInputContexts(NewMenuState, /*out*/ MatchingContexts);
	MatchingContexts.RemoveAll([](const UBmrInputMappingContext* It)
	{
		return It->GetActiveForStates().IsEmpty();
	});
	if (!MatchingContexts.IsEmpty())
	{
		PC.SetupInputContexts(MatchingContexts);
	}
}

/*********************************************************************************************
 * Sounds
 ********************************************************************************************* */

// Trigger the background music to be played in the Main Menu
void UNMMPlayerControllerComponent::PlayMainMenuMusic()
{
	USoundBase* MainMenuMusic = UNMMDataAsset::Get().GetMainMenuMusic();

	if (!MainMenuMusic)
	{
		// Background music is not found, disable current
		StopMainMenuMusic();
		return;
	}

	UBmrSoundsSubsystem::Get().PlaySingleSound2D(MainMenuMusic);
}

// Stops currently played Main Menu background music
void UNMMPlayerControllerComponent::StopMainMenuMusic()
{
	if (USoundBase* MainMenuMusic = UNMMDataAsset::Get().GetMainMenuMusic())
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

	// Clear stale MenuUnloaded from previous session so fresh load does not replay old unload event
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::MenuUnloaded, this);
}

// Clears all transient data created by this component
void UNMMPlayerControllerComponent::OnUnregister()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	// Notify that Main Menu is being unloaded before any cleanup
	ABmrPlayerController* MyPC = GetPlayerController();
	const ABmrPawn* LocalPawn = MyPC ? MyPC->GetPawn<ABmrPawn>() : nullptr;
	if (LocalPawn)
	{
		FGameplayEventData EventData;
		EventData.EventTag = NmmGameplayTags::Event::MenuUnloaded;
		EventData.Instigator = LocalPawn;
		UGlobalMessageSubsystem::BroadcastGlobalMessage(EventData);

		if (!MyPC->HasAuthority())
		{
			MyPC->ServerBroadcastMessage(EventData);
		}
	}

	if (const UNMMDataAsset* DataAsset = UNMMUtils::GetDataAsset())
	{
		// Unregister all input actions and input contexts
		TArray<UBmrInputMappingContext*> MenuInputContexts;
		DataAsset->GetAllInputContexts(/*out*/ MenuInputContexts);

		for (const UBmrInputMappingContext* InputContextIt : MenuInputContexts)
		{
			UInputUtilsLibrary::UnbindInputActionsInContext(MyPC, InputContextIt);
		}
		MyPC->RemoveInputContexts(MenuInputContexts);
	}

	// Cleanup all sounds
	UBmrSoundsSubsystem* SoundSubsystem = UBmrSoundsSubsystem::GetSoundsSubsystem();
	const UNMMDataAsset* SoundDataAsset = SoundSubsystem ? UNMMUtils::GetDataAsset() : nullptr;
	if (USoundBase* MainMenuMusic = SoundDataAsset ? SoundDataAsset->GetMainMenuMusic() : nullptr)
	{
		SoundSubsystem->DestroySingleSound2D(MainMenuMusic);
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
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(NmmGameplayTags::Event::MenuStateChanged, this, &ThisClass::OnNewMainMenuStateChanged);

	BIND_ON_WIDGETS_INITIALIZED(this, ThisClass::OnWidgetsInitialized);

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

// Called when the Main Menu state was changed
void UNMMPlayerControllerComponent::OnNewMainMenuStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const FNmmStateTag NewState(Payload.InstigatorTags.First());
	const bool bIsCinematic = NewState == FNmmStateTag::Cinematic;

	if (bIsCinematic)
	{
		GetPlayerControllerChecked().SetIgnoreMoveInput(true);
		StopMainMenuMusic();
	}

	// Update input contexts
	SetManagedInputContextsEnabled(NewState);

	// Update cinematic input and mouse visibility
	SetCinematicInputContextEnabled(bIsCinematic);
	SetCinematicMouseVisibilityEnabled(bIsCinematic);
}

// Is called when all game widgets are initialized
void UNMMPlayerControllerComponent::OnWidgetsInitialized_Implementation()
{
	// Apply input contexts that might be skipped earlier if widgets were not ready yet
	SetManagedInputContextsEnabled(UNMMUtils::GetMainMenuState());
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
