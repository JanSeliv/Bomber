// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrGlobalEventsSubsystem.h"

// Bomber
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGlobalEventsSubsystem)

// Returns this Subsystem, is checked and will crash if can't be obtained
UBmrGlobalEventsSubsystem& UBmrGlobalEventsSubsystem::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	UBmrGlobalEventsSubsystem* Subsystem = GetGlobalEventsSubsystem(OptionalWorldContext);
	checkf(Subsystem, TEXT("%s: 'Subsystem' is null"), *FString(__FUNCTION__));
	return *Subsystem;
}

// Returns the pointer to this Subsystem
UBmrGlobalEventsSubsystem* UBmrGlobalEventsSubsystem::GetGlobalEventsSubsystem(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UBmrGlobalEventsSubsystem>() : nullptr;
}

// Is called when the Subsystem is created
void UBmrGlobalEventsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	BP_OnGameStateChanged.Clear();

	ReadyHandler.Reset();
	BP_OnPawnReady.Clear();
	BP_OnLocalPawnReady.Clear();
	BP_OnPlayerStateReady.Clear();
	BP_OnLocalPlayerStateReady.Clear();
}