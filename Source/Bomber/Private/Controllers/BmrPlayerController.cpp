// Copyright (c) Yevhenii Selivanov.

#include "Controllers/BmrPlayerController.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrMouseActivityComponent.h"
#include "Components/BmrMoverComponent.h"
#include "DalSubsystem.h"
#include "DataAssets/BmrInputAction.h"
#include "DataAssets/BmrInputMappingContext.h"
#include "DataAssets/BmrPlayerInputDataAsset.h"
#include "FunctionPickerData/FunctionPickerTemplate.h"
#include "GameFramework/BmrCheatManager.h"
#include "GameFramework/BmrGameMode.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrPawnReadySubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/GameFrameworkComponentManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerController)

// Sets default values for this controller's properties
ABmrPlayerController::ABmrPlayerController()
{
	// Set this controller to call the Tick()
	PrimaryActorTick.bCanEverTick = true;

	// Use level 2D-camera without switches
	bAutoManageActiveCameraTarget = false;

	// Set cheat class
	CheatClass = UBmrCheatManager::StaticClass();

	// Create the mouse activity component, so it will be responsible for mouse visibility
	MouseComponent = CreateDefaultSubobject<UBmrMouseActivityComponent>(TEXT("MouseActivityComponent"));

	// Attach to pawn by default, so controller has always valid location: is useful for replication
	bAttachToPawn = true;
}

// Sends a gameplay event to the server via Gameplay Message Router, is useful for actions happening only by client such as UI buttons to start the game etc
void ABmrPlayerController::ServerBroadcastMessage_Implementation(const FGameplayEventData& Payload)
{
	UGlobalMessageSubsystem::BroadcastGlobalMessage(Payload);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// This is called only in the gameplay before calling begin play
void ABmrPlayerController::PostInitializeComponents()
{
	// Before calling the parent, register this controller it can be obtained at very beginning
	if (ABmrGameMode* MyGameMode = UBmrBlueprintFunctionLibrary::GetGameMode())
	{
		MyGameMode->AddPlayerController(this);
	}

	// Register controller to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ABmrPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input focus on the game window
	FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::WindowActivate);

	// Prevents built-in slate input on UMG
	SetUIInputIgnored();

	UDalSubsystem::Get().ListenForDataAsset<UBmrPlayerInputDataAsset>(this, &ThisClass::OnPlayerInputDataAssetLoaded);

#if WITH_EDITOR
	if (GEditor)
	{
		// Listen to handle switch between PIE and SIE (F8) during the game
		FEditorDelegates::OnPreSwitchBeginPIEAndSIE.AddUObject(this, &ThisClass::OnPreSwitchBeginPIEAndSIE);
	}
#endif // WITH_EDITOR
}

// Called when the Player Input data asset is loaded and available
void ABmrPlayerController::OnPlayerInputDataAssetLoaded_Implementation(const UBmrPlayerInputDataAsset* DataAsset)
{
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	// Adds given contexts to the list of auto managed and binds their input actions
	TArray<const UBmrInputMappingContext*> InputContexts;
	DataAsset->GetAllInputContexts(/*out*/ InputContexts);
	SetupInputContexts(InputContexts);
}

// Is overriden to be used when Input System is initialized
void ABmrPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	// Handle UI inputs
	BIND_ON_WIDGETS_INITIALIZED(this, ThisClass::OnWidgetsInitialized);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = UInputUtilsLibrary::GetEnhancedInputSubsystem(this);
	if (ensureMsgf(InputSubsystem, TEXT("ASSERT: [%i] %hs:\n'InputSubsystem' is null!"), __LINE__, __FUNCTION__))
	{
		InputSubsystem->InitalizeUserSettings();
	}

	// Register gameplay mappings, so they can be remapped
	TArray<const UBmrInputMappingContext*> GameplayInputContexts;
	UBmrPlayerInputDataAsset::Get().GetAllGameplayInputContexts(/*out*/ GameplayInputContexts);
	for (const UBmrInputMappingContext* InputContextIt : GameplayInputContexts)
	{
		constexpr bool bRegisterMappings = true;
		UInputUtilsLibrary::SetAllMappingsRegisteredInContext(this, bRegisterMappings, InputContextIt);
	}
}

// Is overriden to notify when this controller possesses new player character
void ABmrPlayerController::OnPossess(APawn* InPawn)
{
	if (HasAuthority())
	{
		Super::OnPossess(InPawn);
	}

	SetControlRotation(FRotator::ZeroRotator);

	// Try to rebind inputs for possessed pawn
	if (IsLocalController())
	{
		ApplyAllInputContexts();
	}

	if (ABmrPlayerState* InPlayerState = GetPlayerState<ABmrPlayerState>())
	{
		InPlayerState->OnPawnChanged(InPawn);
	}

	if (ABmrPawn* BmrPawn = Cast<ABmrPawn>(InPawn))
	{
		UBmrPawnReadySubsystem::Get().Broadcast_OnPawnPossessed(*BmrPawn);
	}
}

// Is overriden to notify the client when this controller possesses new player character
void ABmrPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	OnPossess(GetPawn());
}

// Is overridden to spawn player state or reuse existing one
void ABmrPlayerController::InitPlayerState()
{
	ABmrPlayerState* InPlayerState = UBmrBlueprintFunctionLibrary::GetLocalPlayerState(this);
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

// Is overridden to prevent destroyed possessed pawn, which is expected to be reused
void ABmrPlayerController::PawnLeavingGame()
{
	// Don't call super to avoid destroying the pawn

	if (PlayerState)
	{
		// First, notify the player state (as it might still need valid pawn)
		PlayerState->UnregisterPlayerWithSession();
	}

	if (ABmrPawn* InPawn = GetPawn<ABmrPawn>())
	{
		// Finally, notify the player character
		InPawn->OnPostLogout(this);
	}
}

// Is overridden to stop the game state State Tree when entering Render Movie cinematic mode
void ABmrPlayerController::SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);

	if (bInCinematicMode)
	{
		// Don't even run game state if game is run from the `Render Movie`
		if (ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState())
		{
			MyGameState->StopGameStateTree();
		}
	}
}

// Is overridden to perform cleanup of the controller when it is destroyed
void ABmrPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UBmrPlayerInputDataAsset::Get().EmptyGameplayInputContexts();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when all game widgets are initialized
void ABmrPlayerController::OnWidgetsInitialized_Implementation()
{
	// Listens to handle input on opening and closing the Settings widget
	if (USettingsWidget* SettingsWidget = UBmrBlueprintFunctionLibrary::GetSettingsWidget())
	{
		SettingsWidget->OnToggledSettings.AddUniqueDynamic(this, &ThisClass::OnToggledSettings);
	}

	// Try to rebind widget inputs
	ApplyAllInputContexts();
}

// Listen to toggle movement input
void ABmrPlayerController::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	ApplyAllInputContexts();
}

// Listens to handle input on opening and closing the Settings widget
void ABmrPlayerController::OnToggledSettings_Implementation(bool bIsVisible)
{
	if (ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::Menu))
	{
		// Toggle all previous Input Contexts
		SetAllInputContextsEnabled(!bIsVisible, FBmrGameStateTag::Menu);
	}
	else if (ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::InGame))
	{
		// Toggle In-Game Menu Input Context
		SetInputContextEnabled(!bIsVisible, UBmrPlayerInputDataAsset::Get().GetInGameMenuInputContext());
	}

	// Turn on or off specific Settings input context (it does not contain any game state)
	SetInputContextEnabled(bIsVisible, UBmrPlayerInputDataAsset::Get().GetSettingsInputContext());
}

/*********************************************************************************************
 * Inputs management
 ********************************************************************************************* */

// Returns true if Player Controller is ready to setup all the inputs
bool ABmrPlayerController::CanBindInputActions() const
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem();
	if (!IsLocalController()
	    || !ensureMsgf(WidgetsSubsystem, TEXT("ASSERT: [%i] %hs:\n'WidgetsSubsystem' condition is FALSE"), __LINE__, __FUNCTION__)
	    || !WidgetsSubsystem->AreWidgetInitialized())
	{
		return false;
	}

	const bool bIsStartingState = ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::Menu);
	if (!GetPawn() && bIsStartingState)
	{
		// While in menu (or initializing), player has to be possessed to bind inputs
		return false;
	}

	// --- Add the rest of the conditions here

	return true;
}

// Adds given contexts to the list of auto managed and binds their input actions
void ABmrPlayerController::SetupInputContexts(const TArray<UBmrInputMappingContext*>& InputContexts)
{
	TArray<const UBmrInputMappingContext*> ConstInputContexts;
	ConstInputContexts.Reserve(InputContexts.Num());
	for (const UBmrInputMappingContext* InputContext : InputContexts)
	{
		ConstInputContexts.Emplace(InputContext);
	}
	SetupInputContexts(ConstInputContexts);
}

// Adds given contexts to the list of auto managed and binds their input actions
void ABmrPlayerController::SetupInputContexts(const TArray<const UBmrInputMappingContext*>& InputContexts)
{
	if (!IsLocalController()
	    || !ensureMsgf(!InputContexts.IsEmpty(), TEXT("ASSERT: [%i] %s:\n'InputContexts' is empty"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Add input contexts to the list to be auto turned of or on according current game state
	AddNewInputContexts(InputContexts);

	// Try enable input contexts according current state
	ApplyAllInputContexts();
}

// Removes input contexts from managed list
void ABmrPlayerController::RemoveInputContexts(const TArray<const UBmrInputMappingContext*>& InputContexts)
{
	if (!IsLocalController())
	{
		return;
	}

	for (const UBmrInputMappingContext* InputContextIt : InputContexts)
	{
		if (InputContextIt)
		{
			AllInputContexts.RemoveSwap(InputContextIt);
			SetInputContextEnabled(false, InputContextIt);
		}
	}
}

// Prevents built-in slate input on UMG
void ABmrPlayerController::SetUIInputIgnored()
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
void ABmrPlayerController::SetAllInputContextsEnabled(bool bEnable, FBmrGameStateTag GameStateTag)
{
	for (const UBmrInputMappingContext* InputContextIt : AllInputContexts)
	{
		if (!InputContextIt)
		{
			continue;
		}

		const FGameplayTagContainer& ActiveForStates = InputContextIt->GetActiveForStates();
		if (!ActiveForStates.HasTag(GameStateTag))
		{
			continue;
		}

		SetInputContextEnabled(bEnable, InputContextIt);
	}
}

// Enables all managed input contexts by current game state
void ABmrPlayerController::ApplyAllInputContexts()
{
	for (const UBmrInputMappingContext* InputContextIt : AllInputContexts)
	{
		if (!InputContextIt)
		{
			continue;
		}

		// Enable if current game state matches any of the context's active states
		const FGameplayTagContainer& ActiveForStates = InputContextIt->GetActiveForStates();
		const bool bShouldEnable = !ActiveForStates.IsEmpty() && ABmrGameState::Get().HasAnyMatchingGameplayTags(ActiveForStates);
		SetInputContextEnabled(bShouldEnable, InputContextIt);
	}
}

// Enables or disables specified input context
void ABmrPlayerController::SetInputContextEnabled(bool bEnable, const UBmrInputMappingContext* InInputContext)
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
void ABmrPlayerController::BindInputActionsInContext(const UBmrInputMappingContext* InInputContext)
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
	UInputUtilsLibrary::GetAllActionsInContext(this, InInputContext, EInputActionInContextState::NotBound, /*out*/ InputActions);

	// --- Bind input actions
	for (const UInputAction* InputActionIt : InputActions)
	{
		const UBmrInputAction* ActionIt = Cast<UBmrInputAction>(InputActionIt);
		if (!ActionIt)
		{
			continue;
		}

		for (int32 Index = 0; Index < ActionIt->GetInputActionBindingsNum(); ++Index)
		{
			const FBmrInputActionBinding CurrentBinding = ActionIt->GetInputActionBinding(Index);
			const FName FunctionName = CurrentBinding.FunctionToBind.FunctionName;
			if (!ensureAlwaysMsgf(!FunctionName.IsNone(), TEXT("ASSERT: %s: 'FunctionName' is none, can not bind the action '%s'!"), *FString(__FUNCTION__), *GetNameSafe(ActionIt)))
			{
				continue;
			}

			const FFunctionPicker& StaticContext = CurrentBinding.StaticContext;
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

			const ETriggerEvent TriggerEvent = CurrentBinding.TriggerEvent;
			EnhancedInputComponent->BindAction(ActionIt, TriggerEvent, FoundContextObj, FunctionName);
			UE_LOG(LogBomber, Log, TEXT("Input bound: [%s][%s] %s()->%s()"), *GetNameSafe(InInputContext), *GetNameSafe(InputActionIt), *StaticContext.ToDisplayString(), *FunctionName.ToString());
		}
	}
}

// Adds input contexts to the list to be auto turned of or on according current game state
void ABmrPlayerController::AddNewInputContexts(const TArray<const UBmrInputMappingContext*>& InputContexts)
{
	if (!IsLocalController())
	{
		return;
	}

	for (const UBmrInputMappingContext* InputContextIt : InputContexts)
	{
		if (InputContextIt)
		{
			AllInputContexts.AddUnique(InputContextIt);
		}
	}
}

// Returns the component that responsible for mouse-related logic, or crash if null
UBmrMouseActivityComponent& ABmrPlayerController::GetMouseActivityComponentChecked() const
{
	checkf(MouseComponent, TEXT("ERROR: [%i] %hs:\n'MouseComponent' is null!"), __LINE__, __FUNCTION__);
	return *MouseComponent;
}

/*********************************************************************************************
 * Camera
 ********************************************************************************************* */

// Is overriden to setup camera manager once spawned
void ABmrPlayerController::SpawnPlayerCameraManager()
{
	Super::SpawnPlayerCameraManager();

	// Allow clients to set their own ViewTarget and the server should not replicate it, so each client can view own cinematic
	checkf(PlayerCameraManager, TEXT("ERROR: [%i] %hs:\n'PlayerCameraManager' was not spawned!"), __LINE__, __FUNCTION__);
	PlayerCameraManager->bClientSimulatingViewTarget = true;
}

// Is overriden to return correct camera location and rotation for the player
void ABmrPlayerController::GetPlayerViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	Super::GetPlayerViewPoint(OutLocation, OutRotation);

	const AActor* ViewTarget = PlayerCameraManager ? PlayerCameraManager->GetViewTarget() : nullptr;
	if (!ViewTarget
	    || ViewTarget == this)
	{
		// Camera is not possessed yet, likely game is loading
		// Controller does not have own camera: proper view target is level or pawn
		// Output some far location, so player will not see the level until any camera is possessed
		static const FVector StartupBlackViewLocation(1000000.f);
		OutLocation = StartupBlackViewLocation;
	}

#if !UE_BUILD_SHIPPING
	if (bIsDebugCameraEnabled)
	{
		// Don't use our 2D-camera roll in debug camera to maintain proper rotation in 3D
		OutRotation.Roll = 0.f;
	}
#endif // !UE_BUILD_SHIPPING
}

#if WITH_EDITOR
// Is called in editor by F8 button, when switched between PIE and SIE during the game to handle the Debug Camera
void ABmrPlayerController::OnPreSwitchBeginPIEAndSIE(bool bPIE)
{
	bIsDebugCameraEnabled = !bPIE;
}
#endif