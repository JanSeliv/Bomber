// Copyright (c) Yevhenii Selivanov.

#include "Controllers/MyPlayerController.h"
//---
#include "Bomber.h"
#include "Components/MouseActivityComponent.h"
#include "DataAssets/MyInputAction.h"
#include "DataAssets/MyInputMappingContext.h"
#include "DataAssets/PlayerInputDataAsset.h"
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "UI/InGameMenuWidget.h"
#include "UI/MyHUD.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
///---
#include "EnhancedInputComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
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

// Set the new game state for the current game
void AMyPlayerController::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->ServerSetGameState(NewGameState);
	}
}

// Sets the GameStarting game state
void AMyPlayerController::SetGameStartingState()
{
	ServerSetGameState(ECurrentGameState::GameStarting);
}

// Sets the Menu game state
void AMyPlayerController::SetMenuState()
{
	ServerSetGameState(ECurrentGameState::Menu);
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
		UFunction* FunctionPtr = StaticContext.GetFunction();
		if (!ensureAlwaysMsgf(FunctionPtr, TEXT("ASSERT: [%i] %s:\n'FunctionPtr' is not found in static context: %s, can not bind the action '%s'!"), __LINE__, *FString(__FUNCTION__), *StaticContext.ToDisplayString(), *GetNameSafe(ActionIt)))
		{
			continue;
		}

		UObject* FoundContextObj = nullptr;
		FunctionPtr->ProcessEvent(FunctionPtr, /*Out*/&FoundContextObj);
		if (!ensureAlwaysMsgf(FoundContextObj, TEXT("ASSERT: [%i] %s:\n'FoundContextObj' is not found, next function return nullptr: %s, can not bind the action '%s'!"), __LINE__, *FString(__FUNCTION__), *StaticContext.ToDisplayString(), *GetNameSafe(ActionIt)))
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

// Returns true if Player Controller is ready to setup all the inputs
bool AMyPlayerController::CanBindInputActions() const
{
	if (!IsLocalController())
	{
		return false;
	}

	const AMyHUD* HUD = GetHUD<AMyHUD>();
	if (!HUD || !HUD->AreWidgetInitialized())
	{
		// UI inputs are listen, HUD has to be ready to handle them
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

// Called when an instance of this class is placed (in editor) or spawned
void AMyPlayerController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BindOnGameStateCreated();
}

// This is called only in the gameplay before calling begin play
void AMyPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register controller to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
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
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);

		// Handle current game state if initialized with delay
		if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
		{
			OnGameStateChanged(ECurrentGameState::Menu);
		}
	}

	// Handle UI inputs
	if (AMyHUD* HUD = GetHUD<AMyHUD>())
	{
		if (HUD->AreWidgetInitialized())
		{
			OnWidgetsInitialized();
		}
		else
		{
			HUD->OnWidgetsInitialized.AddDynamic(this, &ThisClass::OnWidgetsInitialized);
		}
	}

	// Adds given contexts to the list of auto managed and binds their input actions
	TArray<const UMyInputMappingContext*> InputContexts;
	UPlayerInputDataAsset::Get().GetAllInputContexts(/*out*/InputContexts);
	SetupInputContexts(InputContexts);
}

// Locks or unlocks movement input
void AMyPlayerController::SetIgnoreMoveInput(bool bShouldIgnore)
{
	// Do not call super to avoid stacking, override it
	IgnoreMoveInput = bShouldIgnore;
}

// Is overriden to notify when this controller possesses new player character
void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetControlRotation(FRotator::ZeroRotator);

	BroadcastOnSetPlayerState();
}

// Is overriden to notify the client when this controller possesses new player character
void AMyPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// Notify client about pawn change
	GetOnNewPawnNotifier().Broadcast(GetPawn());
}

// Is overriden to notify the client when is set new player state
void AMyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	BroadcastOnSetPlayerState();
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
		case ECurrentGameState::Cinematic:
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

	constexpr bool bInvertRest = true;
	SetAllInputContextsEnabled(true, CurrentGameState, bInvertRest);
}

// Listens to handle input on opening and closing the InGame Menu widget
void AMyPlayerController::OnToggledInGameMenu_Implementation(bool bIsVisible)
{
	if (ECurrentGameState::InGame != AMyGameStateBase::GetCurrentGameState())
	{
		// Do not handle input if not in game
		// Note: End-Game state is handled automatically by switching states
		return;
	}

	// Invert gameplay input contexts
	SetAllInputContextsEnabled(!bIsVisible, ECurrentGameState::InGame);

	// Turn on or off specific In-Game menu input context (it does not contain any game state)
	SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());

	checkf(MouseComponentInternal, TEXT("ERROR: [%i] %s:\n'MouseComponentInternal' is null!"), __LINE__, *FString(__FUNCTION__));
	MouseComponentInternal->SetMouseVisibility(bIsVisible);
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

// Is called when all game widgets are initialized
void AMyPlayerController::OnWidgetsInitialized_Implementation()
{
	AMyHUD* HUD = GetHUD<AMyHUD>();
	if (HUD
	    && HUD->OnWidgetsInitialized.IsAlreadyBound(this, &ThisClass::OnWidgetsInitialized))
	{
		HUD->OnWidgetsInitialized.RemoveDynamic(this, &ThisClass::OnWidgetsInitialized);
	}

	// Listens to handle input on opening and closing the InGame Menu widget
	UInGameMenuWidget* InGameMenuWidget = UMyBlueprintFunctionLibrary::GetInGameMenuWidget();
	if (ensureMsgf(InGameMenuWidget, TEXT("ASSERT: 'InGameMenuWidget' is not valid")))
	{
		InGameMenuWidget->OnToggledInGameMenu.AddUniqueDynamic(this, &ThisClass::OnToggledInGameMenu);
	}

	// Listens to handle input on opening and closing the Settings widget
	USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget();
	if (ensureMsgf(SettingsWidget, TEXT("ASSERT: 'SettingsWidget' is not valid")))
	{
		SettingsWidget->OnToggledSettings.AddUniqueDynamic(this, &ThisClass::OnToggledSettings);
	}
}

// Is called on server and on client the player state is set
void AMyPlayerController::BroadcastOnSetPlayerState()
{
	if (!OnSetPlayerState.IsBound())
	{
		return;
	}

	AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>();
	if (!MyPlayerState)
	{
		return;
	}

	OnSetPlayerState.Broadcast(MyPlayerState);
}

// Start listening creating the game state
void AMyPlayerController::BindOnGameStateCreated()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UWorld::FOnGameStateSetEvent& GameStateSetEvent = World->GameStateSetEvent;
	if (!GameStateSetEvent.IsBoundToObject(this))
	{
		GameStateSetEvent.AddUObject(this, &ThisClass::BroadcastOnGameStateCreated);
	}
}

// Is called on server and on client when the game state is initialized
void AMyPlayerController::BroadcastOnGameStateCreated(AGameStateBase* GameState)
{
	AMyGameStateBase* MyGameState = Cast<AMyGameStateBase>(GameState);
	if (!MyGameState)
	{
		return;
	}

	if (OnGameStateCreated.IsBound())
	{
		OnGameStateCreated.Broadcast(MyGameState);
	}
}
