// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrCinematicRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCinematicRow)

// The row that does not contain any data
const FBmrCinematicRow FBmrCinematicRow::Empty = FBmrCinematicRow();

// Returns true if this row is valid
bool FBmrCinematicRow::IsValid() const
{
	return PlayerTag != FBmrPlayerTag::None
	       && LevelSequence != nullptr;
}

// Equal operator
bool FBmrCinematicRow::operator==(const FBmrCinematicRow& Other) const
{
	return PlayerTag == Other.PlayerTag
	       && LevelSequence == Other.LevelSequence;
}