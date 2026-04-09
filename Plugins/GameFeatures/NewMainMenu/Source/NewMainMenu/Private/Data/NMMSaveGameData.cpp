// Copyright (c) Yevhenii Selivanov

#include "Data/NMMSaveGameData.h"

// UE
#include "Kismet/GameplayStatics.h"

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
void UNMMSaveGameData::MarkCinematicAsSeen(int32 CinematicPriority)
{
	if (CinematicPriority == INDEX_NONE)
	{
		return;
	}

	const int32 Bitmask = 1 << CinematicPriority;
	const bool bIsNewCinematic = !(AllSeenCinematicsBitmask & Bitmask);
	if (bIsNewCinematic)
	{
		AllSeenCinematicsBitmask |= Bitmask;
		SaveDataAsync();
	}
}
