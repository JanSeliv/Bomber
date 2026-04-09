// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/SaveGame.h"

#include "NMMSaveGameData.generated.h"

/**
 * Contains the data of New Main Menu needs to be saved.
 */
UCLASS()
class NEWMAINMENU_API UNMMSaveGameData : public USaveGame
{
	GENERATED_BODY()

public:
	/** Returns the name of the save slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	static const FString& GetSaveSlotName();

	/** Returns the name of the save slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	static int32 GetSaveSlotIndex() { return 0; }

	/** Performs the save operation on the background thread. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SaveDataAsync();

	/** Returns true if given cinematic has been seen by player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE bool HasCinematicBeenSeen(int32 CinematicPriority) const { return CinematicPriority != INDEX_NONE && (AllSeenCinematicsBitmask & (1 << CinematicPriority)) != 0; }

	/** Adds given cinematic to the list of cinematics have seen by the player. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void MarkCinematicAsSeen(int32 CinematicPriority);

protected:
	/** Contains a bitmask of priorities in Cinematics table that have been seen by the player.
	 * Is bitmask instead of array since there are less than 32 cinematics in New Main Menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, AdvancedDisplay, Category = "[NewMainMenu]")
	int32 AllSeenCinematicsBitmask = 0;
};
