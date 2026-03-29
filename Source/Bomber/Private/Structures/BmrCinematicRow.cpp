// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrCinematicRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCinematicRow)

// The row that does not contain any data
const FBmrCinematicRow FBmrCinematicRow::Empty = FBmrCinematicRow();

// Returns true if this row is valid
bool FBmrCinematicRow::IsValid() const
{
	return LevelType != ELT::None
	       && PlayerTag != FBmrPlayerTag::None
	       && LevelSequence != nullptr;
}

// Equal operator
bool FBmrCinematicRow::operator==(const FBmrCinematicRow& Other) const
{
	return LevelType == Other.LevelType
	       && PlayerTag == Other.PlayerTag
	       && LevelSequence == Other.LevelSequence;
}