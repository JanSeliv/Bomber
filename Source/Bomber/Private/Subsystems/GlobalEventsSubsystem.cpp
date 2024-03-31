// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GlobalEventsSubsystem.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalEventsSubsystem)

// Returns this Subsystem, is checked and will crash if can't be obtained
UGlobalEventsSubsystem& UGlobalEventsSubsystem::Get()
{
	UGlobalEventsSubsystem* Subsystem = GetGlobalEventsSubsystem();
	checkf(Subsystem, TEXT("%s: 'Subsystem' is null"), *FString(__FUNCTION__));
	return *Subsystem;
}

// Returns the pointer to this Subsystem
UGlobalEventsSubsystem* UGlobalEventsSubsystem::GetGlobalEventsSubsystem(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UGlobalEventsSubsystem>() : nullptr;
}