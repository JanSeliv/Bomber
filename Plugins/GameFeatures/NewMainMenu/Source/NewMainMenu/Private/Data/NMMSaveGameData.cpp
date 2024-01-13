// Copyright (c) Yevhenii Selivanov

#include "Data/NMMSaveGameData.h"
//---
#include "Kismet/GameplayStatics.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMSaveGameData)

// Returns the name of the save slot
const FString& UNMMSaveGameData::GetSaveSlotName()
{
	static const FString SaveSlotName = StaticClass()->GetName();
	return SaveSlotName;
}

// Performs the save operation on the background thread
void UNMMSaveGameData::SaveDataAsync()
{
	UGameplayStatics::AsyncSaveGameToSlot(this, GetSaveSlotName(), GetSaveSlotIndex());
}

// Adds given cinematic to the list of cinematics have seen by the player
void UNMMSaveGameData::MarkCinematicAsSeen(int32 CinematicRowIndex)
{
	const int32 Bitmask = 1 << CinematicRowIndex;
	const bool bIsNewCinematic = !(AllSeenCinematicsBitmaskInternal & Bitmask);
	if (bIsNewCinematic)
	{
		AllSeenCinematicsBitmaskInternal |= Bitmask;
		SaveDataAsync();
	}
}
