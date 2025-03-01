﻿// Copyright (c) Yevhenii Selivanov.

#include "Controllers/MyPlayerController.h"
//---
#include "Bomber.h"
#include "Components/MouseActivityComponent.h"
#include "DataAssets/MyInputAction.h"
#include "DataAssets/MyInputMappingContext.h"
#include "DataAssets/PlayerInputDataAsset.h"
#include "FunctionPickerData/FunctionPickerTemplate.h"
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Subsystems/WidgetsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "EnhancedInputComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
//---
#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyPlayerController)

// Sets default values for this controller's properties
AMyPlayerController::AMyPlayerController()
{
	// Set this controller to call the Tick()
	PrimaryActorTick.bCanEverTick = true;

	// Use level 2D-camera without switches
	bAutoManageActiveCameraTarget = false;

	// Set cheat class
	CheatClass = UMyCheatManager::StaticClass();

	// Create the mouse activity component, so it will be responsible for mouse visibility
	MouseComponentInternal = CreateDefaultSubobject<UMouseActivityComponent>(TEXT("MouseActivityComponent"));
}

/*********************************************************************************************
 * Game States
 * Is designed for clients to change the game state
 * Server can call AMyGameStateBase::Get().SetGameState(NewState) directly
 ********************************************************************************************* */

// Returns true if current game state can be eventually changed
bool AMyPlayerController::CanChangeGameState(ECurrentGameState NewGameState) const
{
	const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	if (!MyGameState
	    || !MyGameState->CanChangeGameState(NewGameState))
	{
		return false;
	}

	// Don't change the game state if game is run from the `Render Movie`
	return !bCinematicMode;
}

// Sets and replicates the Starting game state (3-2-1 countdown), can be called on the client
void AMyPlayerController::SetGameStartingState()
{
	if (CanChangeGameState(ECGS::GameStarting))
	{
		ServerSetGameState(ECGS::GameStarting);
	}
}

// Sets and replicates the Menu game state, can be called on the client
void AMyPlayerController::SetMenuState()
{
	if (CanChangeGameState(ECGS::Menu))
	{
		ServerSetGameState(ECGS::Menu);
	}
}

// Is called during the In-Game state to show results to all players regarding finished match (Win, Lose or Draw)
void AMyPlayerController::SetEndGameState()
{
	if (CanChangeGameState(ECGS::EndGame)
	    && UMyBlueprintFunctionLibrary::GetAlivePlayersNum() <= 1)
	{
		ServerSetGameState(ECGS::EndGame);
	}
}

// Set the new game state for the current game
void AMyPlayerController::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->SetGameState(NewGameState);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Locks or unlocks movement input
void AMyPlayerController::SetIgnoreMoveInput(bool bShouldIgnore)
{
	// Do not call super to avoid stacking, override it
	IgnoreMoveInput = bShouldIgnore;
}

// This is called only in the gameplay before calling begin play
void AMyPlayerController::PostInitializeComponents()
{
	// Before calling the parent, register this controller it can be obtained at very beginning
	if (AMyGameModeBase* MyGameMode = UMyBlueprintFunctionLibrary::GetMyGameMode())
	{
		MyGameMode->AddPlayerController(this);
	}

	// Register controller to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input focus on the game window
	FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::WindowActivate);

	// Prevents built-in slate input on UMG
	SetUIInputIgnored();

	// Listen to handle input for each game state
	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// Adds given contexts to the list of auto managed and binds their input actions
	TArray<const UMyInputMappingContext*> InputContexts;
	UPlayerInputDataAsset::Get().GetAllInputContexts(/*out*/InputContexts);
	SetupInputContexts(InputContexts);

#if WITH_EDITOR
	if (GEditor)
	{
		// Listen to handle switch between PIE and SIE (F8) during the game
		FEditorDelegates::OnPreSwitchBeginPIEAndSIE.AddUObject(this, &ThisClass::OnPreSwitchBeginPIEAndSIE);
	}
#endif // WITH_EDITOR
}

// Is overriden to be used when Input System is initialized
void AMyPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	// Handle UI inputs
	UWidgetsSubsystem* WidgetsSubsystem = UWidgetsSubsystem::GetWidgetsSubsystem(GetLocalPlayer());
	checkf(WidgetsSubsystem, TEXT("ERROR: [%i] %hs:\n'WidgetsSubsystem' is null!"), __LINE__, __FUNCTION__);
	WidgetsSubsystem->OnWidgetsInitialized.AddUniqueDynamic(this, &ThisClass::OnWidgetsInitialized);
	if (WidgetsSubsystem->AreWidgetInitialized())
	{
		OnWidgetsInitialized();
	}

	// Register gameplay mappings, so they can be remapped
	TArray<const UMyInputMappingContext*> GameplayInputContexts;
	UPlayerInputDataAsset::Get().GetAllGameplayInputContexts(/*out*/GameplayInputContexts);
	for (const UMyInputMappingContext* InputContextIt : GameplayInputContexts)
	{
		constexpr bool bRegisterMappings = true;
		UInputUtilsLibrary::SetAllMappingsRegisteredInContext(this, bRegisterMappings, InputContextIt);
	}
}

// Is overriden to notify when this controller possesses new player character
void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetControlRotation(FRotator::ZeroRotator);

	// Try to rebind inputs for possessed pawn on server
	ApplyAllInputContexts();

	// Notify host about pawn change
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InPawn))
	{
		UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.Broadcast_OnCharacterPossessed(*PlayerCharacter);
	}
}

// Is overriden to notify the client when this controller possesses new player character
void AMyPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// Try to rebind inputs for possessed pawn on client
	ApplyAllInputContexts();

	// Notify client about pawn change
	if (APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>())
	{
		UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.Broadcast_OnCharacterPossessed(*PlayerCharacter);
	}
}

// Is overridden to spawn player state or reuse existing one
void AMyPlayerController::InitPlayerState()
{
	AMyPlayerState* InPlayerState = UMyBlueprintFunctionLibrary::GetLocalPlayerState(this);
	if (!InPlayerState)
	{
		// If player state is not found, create a new one
		Super::InitPlayerState();
		return;
	}

	// Set existing player state for this controller
	PlayerState = InPlayerState;
	PlayerState->SetOwner(this);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when all game widgets are initialized
void AMyPlayerController::OnWidgetsInitialized_Implementation()
{
	// Listens to handle input on opening and closing the Settings widget
	USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget();
	if (ensureMsgf(SettingsWidget, TEXT("ASSERT: 'SettingsWidget' is not valid")))
	{
		SettingsWidget->OnToggledSettings.AddUniqueDynamic(this, &ThisClass::OnToggledSettings);
	}

	// Try to rebind widget inputs
	ApplyAllInputContexts();
}

// Listen to toggle movement input
void AMyPlayerController::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			SetIgnoreMoveInput(true);
			break;
		}
		case ECurrentGameState::Menu:
		{
			SetIgnoreMoveInput(true);
			break;
		}
		case ECurrentGameState::EndGame:
		{
			SetIgnoreMoveInput(true);
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetIgnoreMoveInput(false);
			break;
		}
		default:
			break;
	}

	ApplyAllInputContexts();
}

// Listens to handle input on opening and closing the Settings widget
void AMyPlayerController::OnToggledSettings_Implementation(bool bIsVisible)
{
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState();
	if (CurrentGameState == ECGS::Menu)
	{
		// Toggle all previous Input Contexts
		SetAllInputContextsEnabled(!bIsVisible, CurrentGameState);
	}
	else if (CurrentGameState == ECGS::InGame)
	{
		// Toggle In-Game Menu Input Context
		SetInputContextEnabled(!bIsVisible, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());
	}

	// Turn on or off specific Settings input context (it does not contain any game state)
	SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetSettingsInputContext());
}

/*********************************************************************************************
 * Inputs management
 ********************************************************************************************* */

// Returns true if Player Controller is ready to setup all the inputs
bool AMyPlayerController::CanBindInputActions() const
{
	const UWidgetsSubsystem* WidgetsSubsystem = UWidgetsSubsystem::GetWidgetsSubsystem();
	if (!IsLocalController()
	    || !ensureMsgf(WidgetsSubsystem, TEXT("ASSERT: [%i] %hs:\n'WidgetsSubsystem' condition is FALSE"), __LINE__, __FUNCTION__)
	    || !WidgetsSubsystem->AreWidgetInitialized())
	{
		return false;
	}

	const bool bIsStartingState = AMyGameStateBase::GetCurrentGameState() == ECGS::Menu;
	if (!GetPawn() && bIsStartingState)
	{
		// While in menu (or initializing), player has to be possessed to bind inputs
		return false;
	}

	// --- Add the rest of the conditions here

	return true;
}

// Adds given contexts to the list of auto managed and binds their input actions
void AMyPlayerController::SetupInputContexts(const TArray<UMyInputMappingContext*>& InputContexts)
{
	TArray<const UMyInputMappingContext*> ConstInputContexts;
	ConstInputContexts.Reserve(InputContexts.Num());
	for (const UMyInputMappingContext* InputContext : InputContexts)
	{
		ConstInputContexts.Emplace(InputContext);
	}
	SetupInputContexts(ConstInputContexts);
}

// Adds given contexts to the list of auto managed and binds their input actions
void AMyPlayerController::SetupInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts)
{
	if (!IsLocalController()
	    || !ensureMsgf(!InputContexts.IsEmpty(), TEXT("ASSERT: [%i] %s:\n'InputContexts' is empty"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Add input contexts to the list to be auto turned of or on according current game state
	AddNewInputContexts(InputContexts);

	// Try enable input contexts according current state
	constexpr bool bInvertRest = true;
	SetAllInputContextsEnabled(true, AMyGameStateBase::GetCurrentGameState(), bInvertRest);
}

// Removes input contexts from managed list
void AMyPlayerController::RemoveInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts)
{
	if (!IsLocalController())
	{
		return;
	}

	for (const UMyInputMappingContext* InputContextIt : InputContexts)
	{
		if (InputContextIt)
		{
			AllInputContextsInternal.RemoveSwap(InputContextIt);
			SetInputContextEnabled(false, InputContextIt);
		}
	}
}

// Prevents built-in slate input on UMG
void AMyPlayerController::SetUIInputIgnored()
{
	struct FMyNullNavigationConfig : public FNullNavigationConfig
	{
		virtual FORCEINLINE EUINavigation GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const override { return EUINavigation::Invalid; }
		virtual FORCEINLINE EUINavigationAction GetNavigationActionFromKey(const FKeyEvent& InKeyEvent) const override { return EUINavigationAction::Invalid; }
		virtual FORCEINLINE EUINavigation GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) override { return EUINavigation::Invalid; }
	};

	FSlateApplication& SlateApplication = FSlateApplication::Get();
	static const TSharedRef<FNavigationConfig> MyNullNavigationConfig = MakeShared<FMyNullNavigationConfig>();
	if (SlateApplication.GetNavigationConfig() != MyNullNavigationConfig)
	{
		SlateApplication.SetNavigationConfig(MyNullNavigationConfig);
	}
}

// Takes all cached inputs contexts and turns them on or off according given game state
void AMyPlayerController::SetAllInputContextsEnabled(bool bEnable, ECurrentGameState CurrentGameState, bool bInvertRest/* = false*/)
{
	for (const UMyInputMappingContext* InputContextIt : AllInputContextsInternal)
	{
		if (!InputContextIt)
		{
			continue;
		}

		const int32 GameStatesBitmask = InputContextIt->GetChosenGameStatesBitmask();
		const bool bIsForCurrentState = GameStatesBitmask & TO_FLAG(CurrentGameState);
		if (!bIsForCurrentState && !bInvertRest)
		{
			continue;
		}

		const bool bFinalEnable = bIsForCurrentState ? bEnable : !bEnable;
		SetInputContextEnabled(bFinalEnable, InputContextIt);
	}
}

// Enables all managed input contexts by current game state
void AMyPlayerController::ApplyAllInputContexts()
{
	constexpr bool bInvertRest = true;
	SetAllInputContextsEnabled(true, AMyGameStateBase::GetCurrentGameState(), bInvertRest);
}

// Enables or disables specified input context
void AMyPlayerController::SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InInputContext)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %s:\n'InInputContext' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	if (bEnable)
	{
		// Make sure all the input actions are bound
		BindInputActionsInContext(InInputContext);
	}

	UInputUtilsLibrary::SetInputContextEnabled(this, bEnable, InInputContext, InInputContext->GetContextPriority());
}

// Set up input bindings in given contexts
void AMyPlayerController::BindInputActionsInContext(const UMyInputMappingContext* InInputContext)
{
	if (!CanBindInputActions())
	{
		// It could fail on starting the game, but since contexts are managed, it will be bound later anyway
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = UInputUtilsLibrary::GetEnhancedInputComponent(this);
	if (!ensureMsgf(EnhancedInputComponent, TEXT("ASSERT: 'EnhancedInputComponent' is not valid")))
	{
		return;
	}

	// Obtains all input actions in given context that are not currently bound to the input component
	TArray<UInputAction*> InputActions;
	UInputUtilsLibrary::GetAllActionsInContext(this, InInputContext, EInputActionInContextState::NotBound, /*out*/InputActions);

	// --- Bind input actions
	for (const UInputAction* InputActionIt : InputActions)
	{
		const UMyInputAction* ActionIt = Cast<UMyInputAction>(InputActionIt);
		const FName FunctionName = ActionIt ? ActionIt->GetFunctionToBind().FunctionName : NAME_None;
		if (!ensureAlwaysMsgf(!FunctionName.IsNone(), TEXT("ASSERT: %s: 'FunctionName' is none, can not bind the action '%s'!"), *FString(__FUNCTION__), *GetNameSafe(ActionIt)))
		{
			continue;
		}

		const FFunctionPicker& StaticContext = ActionIt->GetStaticContext();
		if (!ensureAlwaysMsgf(StaticContext.IsValid(), TEXT("ASSERT: [%i] %s:\n'StaticContext' is not valid: %s, can not bind the action '%s'!"), __LINE__, *FString(__FUNCTION__), *StaticContext.ToDisplayString(), *GetNameSafe(ActionIt)))
		{
			continue;
		}

		UFunctionPickerTemplate::FOnGetterObject GetOwnerFunc;
		GetOwnerFunc.BindUFunction(StaticContext.FunctionClass->GetDefaultObject(), StaticContext.FunctionName);
		UObject* FoundContextObj = GetOwnerFunc.Execute(GetWorld());
		if (!ensureAlwaysMsgf(FoundContextObj, TEXT("ASSERT: [%i] %s:\n'FoundContextObj' is not found, next function returns nullptr: %s, can not bind the action '%s'!"), __LINE__, *FString(__FUNCTION__), *StaticContext.ToDisplayString(), *GetNameSafe(ActionIt)))
		{
			continue;
		}

		const ETriggerEvent TriggerEvent = ActionIt->GetTriggerEvent();
		EnhancedInputComponent->BindAction(ActionIt, TriggerEvent, FoundContextObj, FunctionName);
		UE_LOG(LogBomber, Log, TEXT("Input bound: [%s][%s] %s()->%s()"), *GetNameSafe(InInputContext), *GetNameSafe(InputActionIt), *StaticContext.ToDisplayString(), *FunctionName.ToString());
	}
}

// Adds input contexts to the list to be auto turned of or on according current game state
void AMyPlayerController::AddNewInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts)
{
	if (!IsLocalController())
	{
		return;
	}

	for (const UMyInputMappingContext* InputContextIt : InputContexts)
	{
		if (InputContextIt)
		{
			AllInputContextsInternal.AddUnique(InputContextIt);
		}
	}
}

// Returns the component that responsible for mouse-related logic, or crash if null
UMouseActivityComponent& AMyPlayerController::GetMouseActivityComponentChecked() const
{
	checkf(MouseComponentInternal, TEXT("ERROR: [%i] %hs:\n'MouseComponentInternal' is null!"), __LINE__, __FUNCTION__);
	return *MouseComponentInternal;
}

/*********************************************************************************************
 * Camera
 ********************************************************************************************* */

// Is overriden to setup camera manager once spawned
void AMyPlayerController::SpawnPlayerCameraManager()
{
	Super::SpawnPlayerCameraManager();

	// Allow clients to set their own ViewTarget and the server should not replicate it, so each client can view own cinematic
	checkf(PlayerCameraManager, TEXT("ERROR: [%i] %hs:\n'PlayerCameraManager' was not spawned!"), __LINE__, __FUNCTION__);
	PlayerCameraManager->bClientSimulatingViewTarget = true;
}

// Is overriden to return correct camera location and rotation for the player
void AMyPlayerController::GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const
{
	Super::GetPlayerViewPoint(Location, Rotation);

#if !UE_BUILD_SHIPPING
	if (bIsDebugCameraEnabledInternal)
	{
		// Don't use our 2D-camera roll in debug camera to maintain proper rotation in 3D 
		Rotation.Roll = 0.f;
	}
#endif // !UE_BUILD_SHIPPING
}

#if WITH_EDITOR
// Is called in editor by F8 button, when switched between PIE and SIE during the game to handle the Debug Camera
void AMyPlayerController::OnPreSwitchBeginPIEAndSIE(bool bPIE)
{
	bIsDebugCameraEnabledInternal = !bPIE;
}
#endif