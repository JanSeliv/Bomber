﻿// Copyright (c) Yevhenii Selivanov.

#include "Controllers/MyPlayerController.h"
//---
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "Globals/MyInputMappingContext.h"
#include "Framework/Application/NavigationConfig.h"
#include "Engine/LocalPlayer.h"
//---
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/MyInputAction.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
#include "UI/InGameMenuWidget.h"
#include "UI/MainMenuWidget.h"
#include "UI/MyHUD.h"
#include "UI/SettingsWidget.h"

// Returns the player input data asset
const UPlayerInputDataAsset& UPlayerInputDataAsset::Get()
{
	const UPlayerInputDataAsset* PlayerInputDataAsset = USingletonLibrary::GetPlayerInputDataAsset();
	checkf(PlayerInputDataAsset, TEXT("The Player Input Data Asset is not valid"))
	return *PlayerInputDataAsset;
}

// Returns all input contexts contained in this data asset
void UPlayerInputDataAsset::GetAllInputContexts(TArray<const UMyInputMappingContext*>& OutInputContexts) const
{
	static constexpr int32 FirstPlayer = 0;
	if (const UMyInputMappingContext* GameplayInputContextP1 = GetGameplayInputContext(FirstPlayer))
	{
		OutInputContexts.Emplace(GameplayInputContextP1);
	}

	static constexpr int32 SecondPlayer = 1;
	if (const UMyInputMappingContext* GameplayInputContextP2 = GetGameplayInputContext(SecondPlayer))
	{
		OutInputContexts.Emplace(GameplayInputContextP2);
	}

	if (const UMyInputMappingContext* MainMenuInputContext = GetMainMenuInputContext())
	{
		OutInputContexts.Emplace(MainMenuInputContext);
	}

	if (const UMyInputMappingContext* InGameMenuInputContext = GetInGameMenuInputContext())
	{
		OutInputContexts.Emplace(InGameMenuInputContext);
	}

	if (const UMyInputMappingContext* SettingsInputContext = GetSettingsInputContext())
	{
		OutInputContexts.Emplace(SettingsInputContext);
	}
}

// Returns the Enhanced Input Mapping Context of gameplay actions for specified local player
const UMyInputMappingContext* UPlayerInputDataAsset::GetGameplayInputContext(int32 LocalPlayerIndex) const
{
	TryCreateGameplayInputContexts();
	return GameplayInputContextsInternal.IsValidIndex(LocalPlayerIndex) ? GameplayInputContextsInternal[LocalPlayerIndex] : nullptr;
}

// Returns true if specified key is mapped to any gameplay input context
bool UPlayerInputDataAsset::IsMappedKey(const FKey& Key) const
{
	return GameplayInputContextsInternal.ContainsByPredicate([&Key](const UMyInputMappingContext* ContextIt)
	{
		return ContextIt && ContextIt->GetMappings().ContainsByPredicate([&Key](const FEnhancedActionKeyMapping& MappingIt)
		{
			return MappingIt.Key == Key;
		});
	});
}

// Creates new contexts if is needed
void UPlayerInputDataAsset::TryCreateGameplayInputContexts() const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Do not create input contexts since the game is not started yet
		return;
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

	// Create new context if any is null
	const int32 ClassesNum = GameplayInputContextClassesInternal.Num();
	for (int32 Index = 0; Index < ClassesNum; ++Index)
	{
		const bool bIsValidIndex = GameplayInputContextsInternal.IsValidIndex(Index);
		const UMyInputMappingContext* GameplayInputContextsIt = bIsValidIndex ? GameplayInputContextsInternal[Index] : nullptr;
		if (GameplayInputContextsIt)
		{
			// Is already created
			continue;
		}

		// Initialize new gameplay contexts
		UWorld* World = USingletonLibrary::Get().GetWorld();
		const TSubclassOf<UMyInputMappingContext>& ContextClassIt = GameplayInputContextClassesInternal[Index];
		if (!World
		    || !ContextClassIt)
		{
			// Is empty class
			continue;
		}

		const FName ContextClassName(*FString::Printf(TEXT("%s_%i"), *ContextClassIt->GetName(), Index));
		UMyInputMappingContext* NewGameplayInputContext = NewObject<UMyInputMappingContext>(World, ContextClassIt, ContextClassName, RF_Public | RF_Transactional);

		if (bIsValidIndex)
		{
			GameplayInputContextsInternal[Index] = NewGameplayInputContext;
		}
		else
		{
			GameplayInputContextsInternal.EmplaceAt(Index, NewGameplayInputContext);
		}
	}
}

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
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
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
	switch (AMyGameStateBase::GetCurrentGameState(this))
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
	if (USingletonLibrary::IsEditorMultiplayer())
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

// Called when an instance of this class is placed (in editor) or spawned
void AMyPlayerController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BindOnGameStateCreated();
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
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
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

	BindInputActions();
}

// Is overriden to notify the client when this controller possesses new player character
void AMyPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// Notify client about pawn change
	GetOnNewPawnNotifier().Broadcast(GetPawn());

	BindInputActions();
}

// Is overriden to notify the client when is set new player state
void AMyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	BroadcastOnSetPlayerState();
}

// Allows the PlayerController to set up custom input bindings
void AMyPlayerController::BindInputActions()
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

	if (EnhancedInputComponent->HasBindings())
	{
		// Remove all previous bindings to do not have duplicates
		EnhancedInputComponent->ClearActionEventBindings();
	}

	TArray<const UMyInputMappingContext*> InputContexts;
	UPlayerInputDataAsset::Get().GetAllInputContexts(InputContexts);

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

	// Enable or disable input contexts by specified game states
	TArray<const UMyInputMappingContext*> InputContexts;
	UPlayerInputDataAsset::Get().GetAllInputContexts(InputContexts);
	for (const UMyInputMappingContext* InputContextIt : InputContexts)
	{
		if (InputContextIt)
		{
			const int32 GameStatesBitmask = InputContextIt->GetChosenGameStatesBitmask();
			const bool bEnableContext = GameStatesBitmask & TO_FLAG(CurrentGameState);
			SetInputContextEnabled(bEnableContext, InputContextIt);
		}
	}
}

// Listens to handle input on opening and closing the InGame Menu widget
void AMyPlayerController::OnToggledInGameMenu(bool bIsVisible)
{
	if (ECurrentGameState::InGame != AMyGameStateBase::GetCurrentGameState(this))
	{
		return;
	}

	SetGameplayInputContextEnabled(!bIsVisible);
	SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());

	SetMouseVisibility(bIsVisible);
}

// Listens to handle input on opening and closing the Settings widget
void AMyPlayerController::OnToggledSettings(bool bIsVisible)
{
	// Toggle Settings Input Context
	SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetSettingsInputContext());

	// Toggle previous Input Context
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState(this);
	const UMyInputMappingContext* PreviousInputContext = nullptr;
	if (CurrentGameState == ECGS::Menu)
	{
		PreviousInputContext = UPlayerInputDataAsset::Get().GetMainMenuInputContext();
	}
	else if (CurrentGameState == ECGS::InGame)
	{
		PreviousInputContext = UPlayerInputDataAsset::Get().GetInGameMenuInputContext();
	}

	if (PreviousInputContext)
	{
		SetInputContextEnabled(!bIsVisible, PreviousInputContext);
	}
}

// Enables or disables input contexts of gameplay input actions
void AMyPlayerController::SetGameplayInputContextEnabled(bool bEnable)
{
	static constexpr int32 LocalPlayers = 2;
	for (int32 PlayerIndex = 0; PlayerIndex < LocalPlayers; ++PlayerIndex)
	{
		const UMyInputMappingContext* InputContextIt = UPlayerInputDataAsset::Get().GetGameplayInputContext(PlayerIndex);
		SetInputContextEnabled(bEnable, InputContextIt);
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

// Is called when all game widgets are initialized
void AMyPlayerController::OnWidgetsInitialized()
{
	AMyHUD* HUD = GetHUD<AMyHUD>();
	if (HUD
	    && HUD->OnWidgetsInitialized.IsAlreadyBound(this, &ThisClass::OnWidgetsInitialized))
	{
		HUD->OnWidgetsInitialized.RemoveDynamic(this, &ThisClass::OnWidgetsInitialized);
	}

	UMainMenuWidget* MainMenuWidget = USingletonLibrary::GetMainMenuWidget();
	if (ensureMsgf(MainMenuWidget, TEXT("ASSERT: 'MainMenuWidget' is not valid")))
	{
		// Update the Menu State
		if (MainMenuWidget->IsReadyMainMenu())
		{
			SetMenuState();
		}
		else
		{
			// Listens to set menu state when menu is ready
			MainMenuWidget->OnMainMenuReady.AddUniqueDynamic(this, &ThisClass::SetMenuState);
		}
	}

	// Listens to handle input on opening and closing the InGame Menu widget
	UInGameMenuWidget* InGameMenuWidget = USingletonLibrary::GetInGameMenuWidget();
	if (ensureMsgf(InGameMenuWidget, TEXT("ASSERT: 'InGameMenuWidget' is not valid")))
	{
		InGameMenuWidget->OnToggledInGameMenu.AddUniqueDynamic(this, &ThisClass::OnToggledInGameMenu);
	}

	// Listens to handle input on opening and closing the Settings widget
	USettingsWidget* SettingsWidget = USingletonLibrary::GetSettingsWidget();
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
