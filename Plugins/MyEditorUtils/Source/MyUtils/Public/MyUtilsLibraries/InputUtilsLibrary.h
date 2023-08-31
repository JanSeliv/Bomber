// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "InputUtilsLibrary.generated.h"

struct FEnhancedActionKeyMapping;
struct FKey;

class APlayerController;
class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedInputComponent;
class UEnhancedPlayerInput;
class UInputMappingContext;
class UInputAction;

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
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(const UObject* WorldContext);

	/** Returns the Enhanced Input Component. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static UEnhancedInputComponent* GetEnhancedInputComponent(const UObject* WorldContext);

	/** Returns the Enhanced Player Input. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static UEnhancedPlayerInput* GetEnhancedPlayerInput(const UObject* WorldContext);

	/*********************************************************************************************
	 * Input Contexts
	 ********************************************************************************************* */
public:
	/** Returns true if specified input context is enabled. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool IsInputContextEnabled(const UObject* WorldContext, const UInputMappingContext* InInputContext);

	/** Enables or disables specified input context. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void SetInputContextEnabled(const UObject* WorldContext, bool bEnable, const UInputMappingContext* InInputContext, int32 Priority = 0);

	/** Returns all input actions set in mappings. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++")
	static void GetAllActionsInContext(const UInputMappingContext* InInputContext, TArray<UInputAction*>& OutInputActions);

	/*********************************************************************************************
	 * Mappings
	 ********************************************************************************************* */
public:
	/** Returns keys mapped to this action in the active input mapping contexts sorted by its priorities. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void GetAllMappingsInAction(const UObject* WorldContext, const UInputAction* InInputAction, TArray<FKey>& OutKeys);

	/** Returns the first mapped key to this action in most priority active input context. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static const FKey& GetFirstMappingInAction(const UObject* WorldContext, const UInputAction* InInputAction);

	/** Returns all mappings where bIsPlayerMappable is true. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++")
	static void GetAllMappingsInContext(const UInputMappingContext* InInputContext, TArray<FEnhancedActionKeyMapping>& OutMappings);

	/** Returns mappings by specified input action.
	 * @param InInputContext Input context to search in.
	 * @param ByInputAction Action by that
	 * @param OutMappings Contains found mappings */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++")
	static void GetMappingsInContextByAction(const UInputMappingContext* InInputContext, const UInputAction* ByInputAction, TArray<FEnhancedActionKeyMapping>& OutMappings);

	/** Returns true if specified key is mapped to given input context.
	 * @param Key The key to check.
	 * @param InInputContext The input context to search in. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Key"))
	static bool IsMappedKeyInContext(const FKey& Key, const UInputMappingContext* InInputContext);

	/** Unmap previous key and map new one.
	 * @param InInputContext The input context to search in.
	 * @param ByInputAction The input action to be remapped.
	 * @param NewKey The key to set.
	 * @param PrevKey The key need to remap.
	 * @return false if something went wrong, f.e the specified key is already mapped to any gameplay context. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "C++", meta = (AutoCreateRefTerm = "NewKey,PrevKey"))
	static bool RemapKeyInContext(UInputMappingContext* InInputContext, const UInputAction* ByInputAction, const FKey& PrevKey, const FKey& NewKey);
	static bool RemapKeyInContext(const UInputMappingContext* InInputContext, const FEnhancedActionKeyMapping& InMapping, const FKey& NewKey);

	/*********************************************************************************************
	 * Internal helpers
	 ********************************************************************************************* */
private:
	/** Private function to get the player controller. */
	static APlayerController* GetLocalPlayerController(const UObject* WorldContext);

	/** Returns true if remapped key is allowed to be saved in config.
	 * Is created to let safe config if only it is packaged build,
	 * so we don't want to save remaps in Editor, it gets serialised right into asset
	 * while in packaged build it should be saved into config file and taken there. */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "C++")
	static bool CanSaveMappingsInConfig();
};
