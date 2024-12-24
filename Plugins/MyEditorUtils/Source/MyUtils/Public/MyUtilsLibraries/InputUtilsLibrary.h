// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "InputUtilsLibrary.generated.h"

class UInputAction;
class UInputMappingContext;

struct FKey;

/**
 * Determines the state of the input action in the context.
 */
UENUM(BlueprintType)
enum class EInputActionInContextState : uint8
{
	///< The input action exists in Input Context, but is not bound to the Player Input 
	NotBound,
	///< The input action exists in Input Context and is bound to the Player Input
	Bound,
	///< The input action exists in Input Context
	Any
};

/**
 * The Enhanced Input functions library.
 * Extends Epic's API with some useful functions and tricks.
 * All the functions are 'BlueprintCosmetic' since they are not intended to be used in on dedicated servers.
 */
UCLASS()
class MYUTILS_API UInputUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Object Getters
	 ********************************************************************************************* */
public:
	/** Returns the Enhanced Input Local Player Subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(const UObject* WorldContext);

	/** Returns the Enhanced Input Component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static class UEnhancedInputComponent* GetEnhancedInputComponent(const UObject* WorldContext);

	/** Returns the Enhanced Player Input. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static class UEnhancedPlayerInput* GetEnhancedPlayerInput(const UObject* WorldContext);

	/** Returns the Enhanced Input User Settings for remapping keys. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static class UEnhancedInputUserSettings* GetEnhancedInputUserSettings(const UObject* WorldContext);

	/*********************************************************************************************
	 * Input Contexts
	 ********************************************************************************************* */
public:
	/** Returns true if specified input context is enabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static bool IsInputContextEnabled(const UObject* WorldContext, const UInputMappingContext* InInputContext);

	/** Enables or disables specified input context. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static void SetInputContextEnabled(const UObject* WorldContext, bool bEnable, const UInputMappingContext* InInputContext, int32 Priority = 0);

	/** Returns all input actions set in mappings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static void GetAllActionsInContext(const UObject* WorldContext, const UInputMappingContext* InInputContext, EInputActionInContextState State, TArray<UInputAction*>& OutInputActions);

	/*********************************************************************************************
	 * Input Actions
	 ********************************************************************************************* */
public:
	/** Returns true if specified input action is bound to the Input Component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static bool IsInputActionBound(const UObject* WorldContext, const UInputAction* InInputAction);

	/*********************************************************************************************
	 * Mappings
	 ********************************************************************************************* */
public:
	/** Returns all mappings where bIsPlayerMappable is true. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static void GetAllMappingsInContext(const UObject* WorldContext, const UInputMappingContext* InInputContext, TArray<struct FPlayerKeyMapping>& OutMappings);

	/** Returns true if specified key is mapped to given input context.
	 * @param WorldContext The world context (controller) to search in.
	 * @param Key The key to check.
	 * @param InInputContext The input context to search in. */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext", AutoCreateRefTerm = "Key"))
	static bool IsMappedKeyInContext(const UObject* WorldContext, const FKey& Key, const UInputMappingContext* InInputContext);

	/** Unmap previous key and map new one.
	 * @param WorldContext The world context (controller) to search in.
	 * @param InInputContext The input context to search in.
	 * @param ByInputAction The input action to be remapped.
	 * @param NewKey The key to set.
	 * @param PrevKey The key need to remap.
	 * @return false if something went wrong, f.e the specified key is already mapped to any gameplay context. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext", AutoCreateRefTerm = "NewKey,PrevKey"))
	static bool RemapKeyInContext(const UObject* WorldContext, const UInputMappingContext* InInputContext, const UInputAction* ByInputAction, const FKey& PrevKey, const FKey& NewKey);

	/** Register or unregister all mappings in the specified input context.
	 * It's required to be called once on those contexts, where user can change mappings.
	 * @param WorldContext The world context (controller) to search in.
	 * @param bRegister If true, then register all mappings, otherwise unregister.
	 * @param InInputContext The input context to register. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext", AutoCreateRefTerm = "NewKey,PrevKey"))
	static void SetAllMappingsRegisteredInContext(const UObject* WorldContext, bool bRegister, const UInputMappingContext* InInputContext);

	/*********************************************************************************************
	 * Internal helpers
	 ********************************************************************************************* */
private:
	/** Private function to get the player controller. */
	static class APlayerController* GetLocalPlayerController(const UObject* WorldContext);
};
