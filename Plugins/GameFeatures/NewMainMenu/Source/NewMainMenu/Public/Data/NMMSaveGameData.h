// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/SaveGame.h"
//---
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
	UFUNCTION(BlueprintPure, Category = "C++")
	static const FString& GetSaveSlotName();

	/** Returns the name of the save slot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetSaveSlotIndex() { return 0; }

	/** Performs the save operation on the background thread. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SaveDataAsync();

	/** Returns true if given cinematic has been seen by player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool HasCinematicBeenSeen(int32 CinematicRowIndex) const { return (AllSeenCinematicsBitmaskInternal & (1 << CinematicRowIndex)) != 0; }

	/** Adds given cinematic to the list of cinematics have seen by the player. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void MarkCinematicAsSeen(int32 CinematicRowIndex);

protected:
	/** Contains a bitmask of row indexes in Cinematics table that have been seen by the player.
	 * Is bitmask instead of array since there are less than 32 cinematics in New Main Menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category = "C++", DisplayName = "Cinematics Played Bitmask")
	int32 AllSeenCinematicsBitmaskInternal = 0;
};
