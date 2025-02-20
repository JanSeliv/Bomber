// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "SaveUtilsLibrary.generated.h"

class USaveGame;

DECLARE_DELEGATE_OneParam(FAsyncLoadGameFromSlot, USaveGame*);

/**
 * Function library with save-related helpers.
 */
UCLASS()
class MYUTILS_API USaveUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Is code alternative of blueprintable UGameplayStatics::AsyncLoadGameFromSlot,
	 * which does the same, but ensures callback will be called in the correct world context, even in PIE multiplayer. */
	static void AsyncLoadGameFromSlot(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex, const FAsyncLoadGameFromSlot& Callback);

	/** Completely removes given save data and creates new empty one.
	 * @param SaveGame The save game to reset.
	 * @param SaveSlotName The name of the save slot.
	 * @param SaveSlotIndex The index of the save slot.
	 * @return The new empty save game object. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static USaveGame* ResetSaveGameData(USaveGame* SaveGame, const FString& SaveSlotName, int32 SaveSlotIndex);

	template <typename T>
	static FORCEINLINE T* ResetSaveGameData(USaveGame* SaveGame, const FString& SaveSlotName, int32 SaveSlotIndex) { return Cast<T>(ResetSaveGameData(SaveGame, SaveSlotName, SaveSlotIndex)); }

	/** Returns true if given object's config properties can be saved to the config file.
	 * Is useful for validations to avoid unexpected behavior in builds.
	 * @param Object The object to check. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool CanSaveConfig(const UObject* Object);

	/** Saves the given object's config properties to the config file.
	 * Is useful because of its validation to avoid unexpected behavior in builds.
	 * @param Object The object to save. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SaveConfig(UObject* Object);
};
