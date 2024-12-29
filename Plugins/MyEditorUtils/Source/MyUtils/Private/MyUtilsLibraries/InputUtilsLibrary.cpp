// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/InputUtilsLibrary.h"
//---
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UserSettings/EnhancedInputUserSettings.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(InputUtilsLibrary)

/*********************************************************************************************
 * Object Getters
 ********************************************************************************************* */

// Returns the Enhanced Input Local Player Subsystem
UEnhancedInputLocalPlayerSubsystem* UInputUtilsLibrary::GetEnhancedInputSubsystem(const UObject* WorldContext)
{
	const APlayerController* PlayerController = GetLocalPlayerController(WorldContext);
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
}

// Returns the Enhanced Input Component
UEnhancedInputComponent* UInputUtilsLibrary::GetEnhancedInputComponent(const UObject* WorldContext)
{
	const APlayerController* PlayerController = GetLocalPlayerController(WorldContext);
	return PlayerController ? Cast<UEnhancedInputComponent>(PlayerController->InputComponent) : nullptr;
}

// Returns the Enhanced Player Input
UEnhancedPlayerInput* UInputUtilsLibrary::GetEnhancedPlayerInput(const UObject* WorldContext)
{
	const APlayerController* PlayerController = GetLocalPlayerController(WorldContext);
	return PlayerController ? Cast<UEnhancedPlayerInput>(PlayerController->PlayerInput) : nullptr;
}

// Returns the Enhanced Input User Settings for remapping keys
class UEnhancedInputUserSettings* UInputUtilsLibrary::GetEnhancedInputUserSettings(const UObject* WorldContext)
{
	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem(WorldContext);
	return InputSubsystem ? InputSubsystem->GetUserSettings() : nullptr;
}

/*********************************************************************************************
 * Input Contexts
 ********************************************************************************************* */

// Returns true if specified input context is enabled
bool UInputUtilsLibrary::IsInputContextEnabled(const UObject* WorldContext, const UInputMappingContext* InInputContext)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %s:\n'InInputContext' is not given!"), __LINE__, *FString(__FUNCTION__)))
	{
		return false;
	}

	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem(WorldContext);
	return InputSubsystem && InputSubsystem->HasMappingContext(InInputContext);
}

// Enables or disables specified input context
void UInputUtilsLibrary::SetInputContextEnabled(const UObject* WorldContext, bool bEnable, const UInputMappingContext* InInputContext, int32 Priority/* = 0*/)
{
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem(WorldContext);
	if (!InputSubsystem)
	{
		// Can be null on remote clients, do nothing
		return;
	}

	if (!ensureMsgf(InInputContext, TEXT("ASSERT: 'InInputContext' is not valid")))
	{
		return;
	}

	if (bEnable)
	{
		InputSubsystem->AddMappingContext(InInputContext, Priority);
	}
	else
	{
		FModifyContextOptions RemoveContextOptions;
		RemoveContextOptions.bIgnoreAllPressedKeysUntilRelease = false;
		RemoveContextOptions.bForceImmediately = true;
		RemoveContextOptions.bNotifyUserSettings = true;
		InputSubsystem->RemoveMappingContext(InInputContext, RemoveContextOptions);
	}
}

// Returns all input actions set in mappings
void UInputUtilsLibrary::GetAllActionsInContext(const UObject* WorldContext, const UInputMappingContext* InInputContext, EInputActionInContextState State, TArray<UInputAction*>& OutInputActions)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %s:\n'InInputContext' is null!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	if (!OutInputActions.IsEmpty())
	{
		OutInputActions.Empty();
	}

	const TArray<FEnhancedActionKeyMapping>& AllMappings = InInputContext->GetMappings();
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		// const_cast to get rid of constness:
		// it's required since 'Action' is init as const, however UE reflection doesn't support array as argument\return value that has const type
		UInputAction* MyInputAction = MappingIt.Action ? const_cast<UInputAction*>(MappingIt.Action.Get()) : nullptr;
		if (!MyInputAction
			|| OutInputActions.Contains(MyInputAction))
		{
			continue;
		}

		bool bIsMatchingState = false;
		switch (State)
		{
		case EInputActionInContextState::NotBound:
			bIsMatchingState = !IsInputActionBound(WorldContext, MyInputAction);
			break;
		case EInputActionInContextState::Bound:
			bIsMatchingState = IsInputActionBound(WorldContext, MyInputAction);
			break;
		case EInputActionInContextState::Any:
			bIsMatchingState = true;
			break;
		default: break;
		}

		if (bIsMatchingState)
		{
			OutInputActions.Emplace(MyInputAction);
		}
	}
}

/*********************************************************************************************
 * Input Actions
 ********************************************************************************************* */

// Returns true if specified input action is bound to the Input Component
bool UInputUtilsLibrary::IsInputActionBound(const UObject* WorldContext, const UInputAction* InInputAction)
{
	const UEnhancedInputComponent* InputComponent = GetEnhancedInputComponent(WorldContext);
	if (!ensureMsgf(InputComponent, TEXT("ASSERT: [%i] %s:\n'InputComponent' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(InInputAction, TEXT("ASSERT: [%i] %s:\n'InInputAction' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return false;
	}

	const TArray<TUniquePtr<FEnhancedInputActionEventBinding>>& AllBoundInputs = InputComponent->GetActionEventBindings();
	return AllBoundInputs.ContainsByPredicate([&InInputAction](const TUniquePtr<FEnhancedInputActionEventBinding>& BindingIt)
	{
		return BindingIt && BindingIt->GetAction() == InInputAction;
	});
}

/*********************************************************************************************
 * Mappings
 ********************************************************************************************* */

// Returns all mappings where bIsPlayerMappable is true
void UInputUtilsLibrary::GetAllMappingsInContext(const UObject* WorldContext, const UInputMappingContext* InInputContext, TArray<FPlayerKeyMapping>& OutMappings)
{
	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem(WorldContext);
	const UEnhancedInputUserSettings* EnhancedInputUserSettings = InputSubsystem ? InputSubsystem->GetUserSettings() : nullptr;
	if (!ensureMsgf(EnhancedInputUserSettings, TEXT("ASSERT: [%i] %hs:\n'EnhancedInputUserSettings' is null, make sure 'Enable User Settings' is enabled in the Project Settings!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %hs:\n'InInputContext' is not valid!"), __LINE__, __FUNCTION__))
	{
		// Can be null on remote clients, do nothing
		return;
	}

	if (!OutMappings.IsEmpty())
	{
		OutMappings.Empty();
	}

	const TArray<FEnhancedActionKeyMapping>& AllMappings = InInputContext->GetMappings();
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		if (!MappingIt.IsPlayerMappable())
		{
			continue;
		}

		const FPlayerKeyMapping* FoundMapping = EnhancedInputUserSettings->FindCurrentMappingForSlot(MappingIt.GetMappingName(), EPlayerMappableKeySlot::First);
		if (!FoundMapping)
		{
			// The key is mappable, but it's not even registered in Enhanced Input User Settings (SetAllMappingsRegisteredInContext is not called)
			continue;
		}

		FPlayerKeyMapping RemappedMapping = *FoundMapping;
		OutMappings.Emplace(MoveTemp(RemappedMapping));
	}
}

// Returns true if specified key is mapped to given input context
bool UInputUtilsLibrary::IsMappedKeyInContext(const UObject* WorldContext, const FKey& Key, const UInputMappingContext* InInputContext)
{
	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem(WorldContext);
	const UEnhancedInputUserSettings* EnhancedInputUserSettings = InputSubsystem ? InputSubsystem->GetUserSettings() : nullptr;
	if (!ensureMsgf(EnhancedInputUserSettings, TEXT("ASSERT: [%i] %hs:\n'EnhancedInputUserSettings' is null, make sure 'Enable User Settings' is enabled in the Project Settings!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %hs:\n'InInputContext' is not valid!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	TArray<FPlayerKeyMapping> AllMappings;
	GetAllMappingsInContext(WorldContext, InInputContext, /*out*/AllMappings);
	return AllMappings.ContainsByPredicate([&Key](const FPlayerKeyMapping& MappingIt)
	{
		return MappingIt.GetCurrentKey() == Key;
	});
}

// Unmap previous key and map new one
bool UInputUtilsLibrary::RemapKeyInContext(const UObject* WorldContextObject, const UInputMappingContext* InInputContext, const UInputAction* ByInputAction, const FKey& PrevKey, const FKey& NewKey)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: 'InInputContext' is not valid"))
		|| !ensureMsgf(ByInputAction, TEXT("ASSERT: [%i] %s:\n'ByInputAction' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| NewKey == PrevKey
		|| IsMappedKeyInContext(WorldContextObject, NewKey, InInputContext))
	{
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetEnhancedInputSubsystem(WorldContextObject);
	UEnhancedInputUserSettings* EnhancedInputUserSettings = InputSubsystem ? InputSubsystem->GetUserSettings() : nullptr;
	if (!ensureMsgf(EnhancedInputUserSettings, TEXT("ASSERT: [%i] %hs:\n'EnhancedInputUserSettings' is null, make sure 'Enable User Settings' is enabled in the Project Settings!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	bool bRemapped = false;
	const TArray<FEnhancedActionKeyMapping>& AllMappings = InInputContext->GetMappings();
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		if (MappingIt.Action != ByInputAction)
		{
			continue;
		}

		FMapPlayerKeyArgs Args;
		Args.MappingName = MappingIt.GetMappingName();
		Args.Slot = EPlayerMappableKeySlot::First;
		Args.NewKey = NewKey;
		Args.bCreateMatchingSlotIfNeeded = true;

		FGameplayTagContainer FailureReason;
		EnhancedInputUserSettings->UnMapPlayerKey(Args, FailureReason);

		EnhancedInputUserSettings->MapPlayerKey(Args, FailureReason);
		bRemapped = FailureReason.IsEmpty();

		break;
	}

	if (!bRemapped)
	{
		return false;
	}

	InputSubsystem->RequestRebuildControlMappings();
	EnhancedInputUserSettings->ApplySettings();
	EnhancedInputUserSettings->AsyncSaveSettings();

	return true;
}

// Register all mappings in the specified contexts
void UInputUtilsLibrary::SetAllMappingsRegisteredInContext(const UObject* WorldContext, bool bRegister, const UInputMappingContext* InInputContext)
{
	UEnhancedInputUserSettings* EnhancedInputUserSettings = GetEnhancedInputUserSettings(WorldContext);
	if (!ensureMsgf(EnhancedInputUserSettings, TEXT("ASSERT: [%i] %hs:\n'EnhancedInputUserSettings' is null, make sure 'Enable User Settings' is enabled in the Project Settings!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (bRegister)
	{
		EnhancedInputUserSettings->RegisterInputMappingContext(InInputContext);
	}
	else
	{
		EnhancedInputUserSettings->UnregisterInputMappingContext(InInputContext);
	}
}

/*********************************************************************************************
 * Internal helpers
 ********************************************************************************************* */

// Private function to get the player controller
APlayerController* UInputUtilsLibrary::GetLocalPlayerController(const UObject* WorldContext)
{
	const APlayerController* PlayerController = Cast<APlayerController>(WorldContext);
	if (PlayerController && PlayerController->IsLocalController())
	{
		return const_cast<APlayerController*>(PlayerController);
	}

	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (World)
	{
		return World->GetFirstPlayerController();
	}

	return nullptr;
}
