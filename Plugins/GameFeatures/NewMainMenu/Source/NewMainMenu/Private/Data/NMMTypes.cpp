// Copyright (c) Yevhenii Selivanov

#include "Data/NMMTypes.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMTypes)

// The row that does not contain any data
const FNMMCinematicRow FNMMCinematicRow::Empty = FNMMCinematicRow();

// Returns true if this row is valid
bool FNMMCinematicRow::IsValid() const
{
	return LevelType != ELT::None
		&& PlayerTag != FPlayerTag::None
		&& LevelSequence != nullptr;
}

// Equal operator
bool FNMMCinematicRow::operator==(const FNMMCinematicRow& Other) const
{
	return LevelType == Other.LevelType
		&& PlayerTag == Other.PlayerTag
		&& LevelSequence == Other.LevelSequence;
}
