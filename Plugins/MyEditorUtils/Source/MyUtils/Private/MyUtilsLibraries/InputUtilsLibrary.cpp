// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/InputUtilsLibrary.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
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
	if (!ensureMsgf(InputSubsystem, TEXT("ASSERT: 'InputSubsystem' is not valid"))
		|| !ensureMsgf(InInputContext, TEXT("ASSERT: 'InInputContext' is not valid")))
	{
		return;
	}

	if (bEnable)
	{
		InputSubsystem->AddMappingContext(InInputContext, Priority);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(InInputContext);
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
		UInputAction* MyInputAction = Cast<UInputAction>(MappingIt.Action);
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

// Returns keys mapped to this action in the active input mapping contexts sorted by its priorities
void UInputUtilsLibrary::GetAllMappingsInAction(const UObject* WorldContext, const UInputAction* InInputAction, TArray<FKey>& OutKeys)
{
	if (!ensureMsgf(InInputAction, TEXT("ASSERT: [%i] %s:\n'InInputAction' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem(WorldContext);
	if (!EnhancedInputSubsystem)
	{
		return;
	}

	if (!OutKeys.IsEmpty())
	{
		OutKeys.Empty();
	}

	OutKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(InInputAction);
}

// Returns the first mapped key to this action in most priority active input context
FKey UInputUtilsLibrary::GetFirstMappingInAction(const UObject* WorldContext, const UInputAction* InInputAction)
{
	TArray<FKey> OutKeys;
	GetAllMappingsInAction(WorldContext, InInputAction, OutKeys);

	constexpr int32 KeyIndex = 0;
	static const FKey EmptyKey{};
	return OutKeys.IsValidIndex(KeyIndex) ? OutKeys[KeyIndex] : EmptyKey;
}

// Returns all mappings where bIsPlayerMappable is true
void UInputUtilsLibrary::GetAllMappingsInContext(const UInputMappingContext* InInputContext, TArray<FEnhancedActionKeyMapping>& OutMappings)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %s:\n'InInputContext' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	if (!OutMappings.IsEmpty())
	{
		OutMappings.Empty();
	}

	const TArray<FEnhancedActionKeyMapping>& AllMappings = InInputContext->GetMappings();
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		if (MappingIt.IsPlayerMappable())
		{
			OutMappings.Emplace(MappingIt);
		}
	}
}

// Returns mappings by specified input action
void UInputUtilsLibrary::GetMappingsInContextByAction(const UInputMappingContext* InInputContext, const UInputAction* ByInputAction, TArray<FEnhancedActionKeyMapping>& OutMappings)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %s:\n'InInputContext' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(ByInputAction, TEXT("ASSERT: [%i] %s:\n'ByInputAction' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	if (!OutMappings.IsEmpty())
	{
		OutMappings.Empty();
	}

	TArray<FEnhancedActionKeyMapping> AllMappings;
	GetAllMappingsInContext(InInputContext, /*out*/AllMappings);
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		if (MappingIt.Action == ByInputAction)
		{
			OutMappings.Emplace(MappingIt);
		}
	}
}

// Returns true if specified key is mapped to given input context
bool UInputUtilsLibrary::IsMappedKeyInContext(const FKey& Key, const UInputMappingContext* InInputContext)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: [%i] %s:\n'InInputContext' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return false;
	}

	TArray<FEnhancedActionKeyMapping> AllMappings;
	GetAllMappingsInContext(InInputContext, /*out*/AllMappings);
	return AllMappings.ContainsByPredicate([&Key](const FEnhancedActionKeyMapping& MappingIt)
	{
		return MappingIt.Key == Key;
	});
}

// Unmap previous key and map new one
bool UInputUtilsLibrary::RemapKeyInContext(UInputMappingContext* InInputContext, const UInputAction* ByInputAction, const FKey& PrevKey, const FKey& NewKey)
{
	if (!ensureMsgf(InInputContext, TEXT("ASSERT: 'InInputContext' is not valid"))
		|| !ensureMsgf(ByInputAction, TEXT("ASSERT: [%i] %s:\n'ByInputAction' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| NewKey == PrevKey
		|| IsMappedKeyInContext(NewKey, InInputContext))
	{
		return false;
	}

	bool bRemapped = false;
	const TArray<FEnhancedActionKeyMapping>& AllMappings = InInputContext->GetMappings();
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		if (MappingIt.Action == ByInputAction
			&& MappingIt.Key == PrevKey)
		{
			InInputContext->UnmapKey(ByInputAction, PrevKey);
			InInputContext->MapKey(ByInputAction, NewKey);
			bRemapped = true;
			break;
		}
	}

	if (!bRemapped)
	{
		return false;
	}

	if (CanSaveMappingsInConfig())
	{
		InInputContext->SaveConfig();
	}

	return true;
}

// Unmap previous key and map new one
bool UInputUtilsLibrary::RemapKeyInContext(const UInputMappingContext* InInputContext, const FEnhancedActionKeyMapping& InMapping, const FKey& NewKey)
{
	UInputMappingContext* ContextToRemap = const_cast<UInputMappingContext*>(InInputContext);
	return RemapKeyInContext(ContextToRemap, InMapping.Action, InMapping.Key, NewKey);
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

// Returns true if remapped key is allowed to be saved in config
bool UInputUtilsLibrary::CanSaveMappingsInConfig()
{
	// Always return true in cook since there remaps should be saved into config file and taken there.
	// We don't want to save remaps in Editor, it gets serialised right into asset
	return !UUtilsLibrary::IsEditor();
}
