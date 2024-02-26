// Copyright (c) Yevhenii Selivanov

#include "Decorators/BTDecorator_HasDifficulty.h"
//---
#include "Bomber.h"
#include "Subsystems/GameDifficultySubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(BTDecorator_HasDifficulty)

// Default constructor
UBTDecorator_HasDifficulty::UBTDecorator_HasDifficulty()
{
	NodeName = TEXT("Has Difficulty");
	bAllowAbortLowerPri = false;
	bAllowAbortNone = false;
	bAllowAbortChildNodes = false;
	FlowAbortMode = EBTFlowAbortMode::None;
}

// Returns true if the game difficulty level is matched with one or more specified types
bool UBTDecorator_HasDifficulty::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return UGameDifficultySubsystem::Get().HasDifficulty(DifficultiesInternal);
}

// Returns string containing the description of the decorator's properties
FString UBTDecorator_HasDifficulty::GetStaticDescription() const
{
	FString Difficulties;
	static const FString Separator = TEXT(", ");
	for (int32 It = 1; It <= TO_FLAG(EGameDifficulty::Any); It <<= 1)
	{
		const EGameDifficulty EnumIt = TO_ENUM(EGameDifficulty, It);
		const EGameDifficulty EnumBitmask = TO_ENUM(EGameDifficulty, DifficultiesInternal);
		if (EnumHasAnyFlags(EnumBitmask, EnumIt))
		{
			Difficulties += UEnum::GetValueOrBitfieldAsString(EnumIt) + Separator;
		}
	}
	Difficulties.RemoveFromEnd(*Separator);

	return Difficulties;
}
