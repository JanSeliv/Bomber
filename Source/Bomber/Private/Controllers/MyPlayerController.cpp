// Copyright (c) Yevhenii Selivanov.

#include "Controllers/MyPlayerController.h"
//---
#include "Bomber.h"
#include "DataAssets/MyInputAction.h"
#include "DataAssets/MyInputMappingContext.h"
#include "DataAssets/PlayerInputDataAsset.h"
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "UI/InGameMenuWidget.h"
#include "UI/MyHUD.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
///---
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
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

// Returns true if the mouse cursor can be hidden
bool AMyPlayerController::CanHideMouse() const
{
	switch (AMyGameStateBase::GetCurrentGameState())
	{
		case ECurrentGameState::GameStarting:
		case ECurrentGameState::InGame:
			return true;
		default:
			return false;
	}
}

// Called to to set mouse cursor visibility
void AMyPlayerController::SetMouseVisibility(bool bShouldShow)
{
	const bool bFailedToHide = !bShouldShow && !CanHideMouse();
	if (bFailedToHide
	    || !IsLocalController())
	{
		return;
	}

	SetShowMouseCursor(bShouldShow);
	bEnableClickEvents = bShouldShow;
	bEnableMouseOverEvents = bShouldShow;
	SetMouseFocusOnUI(bShouldShow);
}

// If true, set the mouse focus on game and UI, otherwise only focusing on game inputs
void AMyPlayerController::SetMouseFocusOnUI(bool bFocusOnUI)
{
#if WITH_EDITOR // [IsEditorMultiplayer]
	if (FEditorUtilsLibrary::IsEditorMultiplayer())
	{
		const ULocalPlayer* LocalPlayer = GetLocalPlayer();
		UGameViewportClient* GameViewport = LocalPlayer ? LocalPlayer->ViewportClient : nullptr;
		FViewport* Viewport = GameViewport ? GameViewport->Viewport : nullptr;
		if (!Viewport
		    || !GameViewport->IsFocused(Viewport))
		{
			// Do not change the focus for inactive viewports in editor-multiplayer
			// to avoid misleading focus on another game window
			return;
		}
	}
#endif // WITH_EDITOR [IsEditorMultiplayer]

	if (bFocusOnUI)
	{
		FInputModeGameAndUI GameAndUI;
		GameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(GameAndUI);
	}
	else
	{
		static const FInputModeGameOnly GameOnly{};
		SetInputMode(GameOnly);
	}
}

// Returns the Enhanced Input Local Player Subsystem
UEnhancedInputLocalPlayerSubsystem* AMyPlayerController::GetEnhancedInputSubsystem() const
{
	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
}

// Returns the Enhanced Input Component
UEnhancedInputComponent* AMyPlayerController::GetEnhancedInputComponent() const
{
	return Cast<UEnhancedInputComponent>(InputComponent);
}

// Returns the Enhanced Player Input
UEnhancedPlayerInput* AMyPlayerController::GetEnhancedPlayerInput() const
{
	return Cast<UEnhancedPlayerInput>(PlayerInput);
}

// Set up input bindings in given contexts
void AMyPlayerController::BindInputActionsInContexts(const TArray<const UMyInputMappingContext*>& InputContexts, bool bClearPreviousBindings/* = false*/)
{
	if (!IsLocalController())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = GetEnhancedInputComponent();
	if (!ensureMsgf(EnhancedInputComponent, TEXT("ASSERT: 'EnhancedInputComponent' is not valid")))
	{
		return;
	}

	if (bClearPreviousBindings
	    && EnhancedInputComponent->HasBindings())
	{
		// Remove all previous bindings to do not have duplicates
		EnhancedInputComponent->ClearActionEventBindings();
	}

	TArray<UMyInputAction*> InputActions;
	for (const UMyInputMappingContext* InputContextIt : InputContexts)
	{
		InputContextIt->GetInputActions(InputActions);
	}

	// --- Bind input actions
	for (const UMyInputAction* ActionIt : InputActions)
	{
		const FName FunctionName = ActionIt ? ActionIt->GetFunctionToBind().FunctionName : NAME_None;
		if (FunctionName.IsNone())
		{
			continue;
		}

		UObject* FoundContextObj = nullptr;
		if (UFunction* FunctionPtr = ActionIt->GetStaticContext().GetFunction())
		{
			FunctionPtr->ProcessEvent(FunctionPtr, /*Out*/&FoundContextObj);
		}

		if (!FoundContextObj)
		{
			continue;
		}

		const ETriggerEvent TriggerEvent = ActionIt->GetTriggerEvent();
		EnhancedInputComponent->BindAction(ActionIt, TriggerEvent, FoundContextObj, FunctionName);
	}
}

// Adds input contexts to the list to be auto turned of or on according current game state
void AMyPlayerController::AddInputContexts(const TArray<const UMyInputMappingContext*>& InputContexts)
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

	// Set mouse focus
	SetMouseFocusOnUI(true);

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
}

// Locks or unlocks movement input
void AMyPlayerController::SetIgnoreMoveInput(bool bShouldIgnore)
{
	// Do not call super to avoid stacking, override it

	if (!bShouldIgnore
	    && !CanHideMouse())
	{
		return;
	}

	SetMouseVisibility(bShouldIgnore);
	IgnoreMoveInput = bShouldIgnore;
}

// Is overriden to notify when this controller possesses new player character
void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetControlRotation(FRotator::ZeroRotator);

	BroadcastOnSetPlayerState();

	SetupPlayerInputs();
}

// Is overriden to notify the client when this controller possesses new player character
void AMyPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// Notify client about pawn change
	GetOnNewPawnNotifier().Broadcast(GetPawn());

	SetupPlayerInputs();
}

// Is overriden to notify the client when is set new player state
void AMyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	BroadcastOnSetPlayerState();
}

// Allows the PlayerController to set up custom input bindings
void AMyPlayerController::SetupPlayerInputs()
{
	TArray<const UMyInputMappingContext*> InputContexts;
	UPlayerInputDataAsset::Get().GetAllInputContexts(InputContexts);

	// Add input contexts to the list to be auto turned of or on according current game state
	AddInputContexts(InputContexts);

	// Bind input actions in all managed contexts
	constexpr bool bClearPreviousBindings = true;
	BindInputActionsInContexts(AllInputContextsInternal, bClearPreviousBindings);

	// Notify other systems that player controller is ready to bind their own input actions
	OnSetupPlayerInputs.Broadcast();
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

// Listen to toggle movement input and mouse cursor
void AMyPlayerController::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			SetIgnoreMoveInput(true);
			SetMouseVisibility(false);
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

	constexpr bool bInvertRest = true;
	SetInputContextsEnabled(true, CurrentGameState, bInvertRest);
}

// Listens to handle input on opening and closing the InGame Menu widget
void AMyPlayerController::OnToggledInGameMenu(bool bIsVisible)
{
	if (ECurrentGameState::InGame != AMyGameStateBase::GetCurrentGameState())
	{
		return;
	}

	// Invert gameplay input contexts
	SetInputContextsEnabled(!bIsVisible, ECurrentGameState::InGame);

	// Turn on or off specific In-Game menu input context (it does not contain any game state)
	SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());

	SetMouseVisibility(bIsVisible);
}

// Listens to handle input on opening and closing the Settings widget
void AMyPlayerController::OnToggledSettings(bool bIsVisible)
{
	// Turn on or off specific Settings input context (it does not contain any game state)
	SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetSettingsInputContext());

	// Toggle previous Input Context
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState();
	if (CurrentGameState == ECGS::Menu || CurrentGameState == ECGS::InGame)
	{
		SetInputContextsEnabled(!bIsVisible, CurrentGameState);
	}
}

// Returns true if specified input context is enabled
bool AMyPlayerController::IsInputContextEnabled(const UMyInputMappingContext* InputContext) const
{
	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem();
	if (!ensureMsgf(InputSubsystem, TEXT("ASSERT: 'InputSubsystem' is not valid"))
	    || !ensureMsgf(InputContext, TEXT("ASSERT: 'InputContext' is not valid")))
	{
		return false;
	}

	return InputSubsystem->HasMappingContext(InputContext);
}

// Enables or disables specified input context
void AMyPlayerController::SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InputContext)
{
	if (!IsLocalController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem();
	if (!ensureMsgf(InputSubsystem, TEXT("ASSERT: 'InputSubsystem' is not valid"))
	    || !ensureMsgf(InputContext, TEXT("ASSERT: 'InputContext' is not valid")))
	{
		return;
	}

	if (bEnable)
	{
		InputSubsystem->AddMappingContext(InputContext, InputContext->GetContextPriority());
	}
	else
	{
		InputSubsystem->RemoveMappingContext(InputContext);
	}
}

// Takes all cached inputs contexts and turns them on or off according given game state
void AMyPlayerController::SetInputContextsEnabled(bool bEnable, ECurrentGameState CurrentGameState, bool bInvertRest/* = false*/)
{
	for (const UMyInputMappingContext* InputContextIt : AllInputContextsInternal)
	{
		if (!InputContextIt)
		{
			continue;
		}

		const int32 GameStatesBitmask = InputContextIt->GetChosenGameStatesBitmask();
		const bool bIsMatching = GameStatesBitmask & TO_FLAG(CurrentGameState);

		if (bIsMatching)
		{
			SetInputContextEnabled(bEnable, InputContextIt);
		}
		else if (bInvertRest)
		{
			SetInputContextEnabled(!bEnable, InputContextIt);
		}
	}
}

// Is called when all game widgets are initialized
void AMyPlayerController::OnWidgetsInitialized()
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
