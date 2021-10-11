// Copyright 2021 Yevhenii Selivanov.

#include "Controllers/MyPlayerController.h"
//---
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Globals/MyInputMappingContext.h"
#include "Framework/Application/NavigationConfig.h"
#include "Engine/LocalPlayer.h"
//---
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/MyInputAction.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
#include "UI/InGameWidget.h"
#include "UI/MainMenuWidget.h"

// Returns the player input data asset
const UPlayerInputDataAsset& UPlayerInputDataAsset::Get()
{
	const UPlayerInputDataAsset* PlayerInputDataAsset = USingletonLibrary::GetPlayerInputDataAsset();
	checkf(PlayerInputDataAsset, TEXT("The Player Input Data Asset is not valid"))
	return *PlayerInputDataAsset;
}

// Returns the Enhanced Input Mapping Context of gameplay actions for specified local player
UMyInputMappingContext* UPlayerInputDataAsset::GetGameplayInputContext(int32 LocalPlayerIndex) const
{
	const TArray<TObjectPtr<UMyInputMappingContext>>& InputContexts = GameplayInputContextsInternal;
	return InputContexts.IsValidIndex(LocalPlayerIndex) ? InputContexts[LocalPlayerIndex] : nullptr;
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
	if (!bShouldShow
	    && !CanHideMouse())
	{
		return;
	}

	bShowMouseCursor = bShouldShow;
	bEnableClickEvents = bShouldShow;
	bEnableMouseOverEvents = bShouldShow;
}

// Returns the Enhanced Input Local Player Subsystem
UEnhancedInputLocalPlayerSubsystem* AMyPlayerController::GetEnhancedInputSubsystem() const
{
	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
}

// Finds input actions in specified contexts
void AMyPlayerController::GetInputActions(TArray<UMyInputAction*>& OutInputActions, const TArray<UMyInputMappingContext*>& InputContexts) const
{
	for (const UMyInputMappingContext* GameplayInputContextIt : InputContexts)
	{
		if (!ensureMsgf(GameplayInputContextIt, TEXT("ASSERT: 'GameplayInputContextIt' is not valid")))
		{
			continue;
		}

		const TArray<FEnhancedActionKeyMapping>& Mappings = GameplayInputContextIt->GetMappings();
		for (const FEnhancedActionKeyMapping& MappingIt : Mappings)
		{
			const UMyInputAction* MyInputAction = Cast<UMyInputAction>(MappingIt.Action);
			if (MyInputAction)
			{
				OutInputActions.AddUnique(const_cast<UMyInputAction*>(MyInputAction));
			}
		}
	}
}

// Called when the game starts or when spawned
void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BindInputActions();

	// Set input focus on the game
	FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::WindowActivate);

	// Prevents built-in slate input on UMG
	SetUIInputIgnored();

	// Listen to handle input for each game state
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	// Update the Menu State
	if (UMainMenuWidget* MainMenuWidget = USingletonLibrary::GetMainMenuWidget())
	{
		if (MainMenuWidget->IsReadyMainMenu())
		{
			SetMenuState();
		}
		else
		{
			// Listens to set menu state when menu is ready
			MainMenuWidget->OnMainMenuReady.AddDynamic(this, &ThisClass::SetMenuState);
		}
	}

	// Listens to handle input on opening and closing the InGame Menu widget
	if (UInGameWidget* InGameWidget = USingletonLibrary::GetInGameWidget())
	{
		InGameWidget->OnToggledInGameMenu.AddDynamic(this, &ThisClass::OnToggledInGameMenu);
	}

	ExecuteDefaultConsoleCommands();
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

// Overridable native function for when this controller unpossesses its pawn
void AMyPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	SetIgnoreMoveInput(true);
}

// Allows the PlayerController to set up custom input bindings
void AMyPlayerController::BindInputActions()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!ensureMsgf(EnhancedInputComponent, TEXT("ASSERT: 'EnhancedInputComponent' is not valid")))
	{
		return;
	}

	UMyInputMappingContext* InputContextP0 = UPlayerInputDataAsset::Get().GetGameplayInputContext(0);
	UMyInputMappingContext* InputContextP1 = UPlayerInputDataAsset::Get().GetGameplayInputContext(1);
	UMyInputMappingContext* MainMenu = UPlayerInputDataAsset::Get().GetMainMenuInputContext();
	UMyInputMappingContext* InGameMenu = UPlayerInputDataAsset::Get().GetInGameMenuInputContext();
	const TArray<UMyInputMappingContext*> InputContexts{InputContextP0, InputContextP1, MainMenu, InGameMenu};

	// --- Bind input actions
	TArray<UMyInputAction*> InputActions;
	GetInputActions(/*Out*/InputActions, InputContexts);
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
			FunctionPtr->ProcessEvent(FunctionPtr, &FoundContextObj);
		}

		// Bind action
		if (FoundContextObj)
		{
			const ETriggerEvent TriggerEvent = ActionIt->GetTriggerEvent();
			EnhancedInputComponent->BindAction(ActionIt, TriggerEvent, FoundContextObj, FunctionName);
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

// Listen to toggle movement input and mouse cursor
void AMyPlayerController::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	// @TODO: Set contexts by game states
	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			SetMouseVisibility(false);
			SetGameplayInputContextEnabled(false);
			SetInputContextEnabled(false, UPlayerInputDataAsset::Get().GetMainMenuInputContext());
			SetInputContextEnabled(false, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());
			break;
		}
		case ECurrentGameState::Menu:
		{
			SetMouseVisibility(true);
			SetGameplayInputContextEnabled(false);
			SetInputContextEnabled(true, UPlayerInputDataAsset::Get().GetMainMenuInputContext());
			SetInputContextEnabled(false, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());
			break;
		}
		case ECurrentGameState::EndGame:
		{
			SetMouseVisibility(true);
			SetGameplayInputContextEnabled(false);
			SetInputContextEnabled(false, UPlayerInputDataAsset::Get().GetMainMenuInputContext());
			SetInputContextEnabled(true, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetMouseVisibility(false);
			SetGameplayInputContextEnabled(true);
			SetInputContextEnabled(false, UPlayerInputDataAsset::Get().GetMainMenuInputContext());
			SetInputContextEnabled(false, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());
			break;
		}
		default:
			break;
	}
}

// Listens to handle input on opening and closing the InGame Menu widget
void AMyPlayerController::OnToggledInGameMenu(bool bIsVisible)
{
	if (ECurrentGameState::InGame == AMyGameStateBase::GetCurrentGameState(GetWorld()))
	{
		SetGameplayInputContextEnabled(!bIsVisible);
		SetInputContextEnabled(bIsVisible, UPlayerInputDataAsset::Get().GetInGameMenuInputContext());
	}
}

// Called default console commands on begin play
void AMyPlayerController::ExecuteDefaultConsoleCommands()
{
	static const FString NaniteDisableCommand(TEXT("r.Nanite 0"));
	ConsoleCommand(NaniteDisableCommand);
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

// Enables or disables specified input context
void AMyPlayerController::SetInputContextEnabled(bool bEnable, const UMyInputMappingContext* InputContext)
{
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem();
	if (!ensureMsgf(InputSubsystem, TEXT("ASSERT: 'InputSubsystem' is not valid"))
	    || !ensureMsgf(InputContext, TEXT("ASSERT: 'InputContext' is not valid")))
	{
		return;
	}

	if (bEnable)
	{
		static constexpr int32 Priority = 0;
		InputSubsystem->AddMappingContext(InputContext, Priority);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(InputContext);
	}
}

// Move the player character by the forward vector
void AMyPlayerController::MoveUpDown(const FInputActionValue& ActionValue)
{
	if (APlayerCharacter* PlayerCharacter = USingletonLibrary::GetControllablePlayer())
	{
		const float ScaleValue = ActionValue.GetMagnitude();
		const FVector RightVector(PlayerCharacter->GetActorRightVector());
		PlayerCharacter->AddMovementInput(RightVector, ScaleValue);
	}
}

// Move the player character by the right vector.
void AMyPlayerController::MoveRightLeft(const FInputActionValue& ActionValue)
{
	if (APlayerCharacter* PlayerCharacter = USingletonLibrary::GetControllablePlayer())
	{
		const float ScaleValue = ActionValue.GetMagnitude();
		const FVector ForwardVector(PlayerCharacter->GetActorForwardVector());
		PlayerCharacter->AddMovementInput(ForwardVector, ScaleValue);
	}
}

// Executes spawning the bomb on controllable player
void AMyPlayerController::SpawnBomb()
{
	if (APlayerCharacter* PlayerCharacter = USingletonLibrary::GetControllablePlayer())
	{
		PlayerCharacter->SpawnBomb();
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
