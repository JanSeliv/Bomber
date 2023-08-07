// Copyright (c) Yevhenii Selivanov

#include "Data/NewMainMenuTypes.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuTypes)

// The row that does not contain any data
const FCinematicRow FCinematicRow::Empty = FCinematicRow();

// Returns true if this row is valid
bool FCinematicRow::IsValid() const
{
	return LevelType != ELT::None
		&& PlayerTag != FPlayerTag::None
		&& LevelSequence != nullptr;
}

// Equal operator
bool FCinematicRow::operator==(const FCinematicRow& Other) const
{
	return LevelType == Other.LevelType
		&& PlayerTag == Other.PlayerTag
		&& LevelSequence == Other.LevelSequence;
}
