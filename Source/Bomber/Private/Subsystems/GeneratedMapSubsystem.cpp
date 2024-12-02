// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GeneratedMapSubsystem.h"
//---
#include "GeneratedMap.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/World.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GeneratedMapSubsystem)

// Returns the Generated Map Subsystem, is checked and will crash if can't be obtained
UGeneratedMapSubsystem& UGeneratedMapSubsystem::Get(const UObject* WorldContextObject/* = nullptr*/)
{
	UGeneratedMapSubsystem* GeneratedMapSubsystem = GetGeneratedMapSubsystem(WorldContextObject);
	checkf(GeneratedMapSubsystem, TEXT("%s: 'GeneratedMapSubsystem' is null"), *FString(__FUNCTION__));
	return *GeneratedMapSubsystem;
}

// Returns the pointer to the Generated Map Subsystem
UGeneratedMapSubsystem* UGeneratedMapSubsystem::GetGeneratedMapSubsystem(const UObject* WorldContextObject/* = nullptr*/)
{
	const UWorld* FoundWorld = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	return FoundWorld ? FoundWorld->GetSubsystem<UGeneratedMapSubsystem>() : nullptr;
}

// The Generated Map getter, nullptr otherwise
AGeneratedMap* UGeneratedMapSubsystem::GetGeneratedMap(bool bWarnIfNull/* = true*/) const
{
#if WITH_EDITOR
	if (bWarnIfNull)
	{
		ensureMsgf(FEditorUtilsLibrary::IsCooking() || GeneratedMapInternal, TEXT("%s: [Editor] 'GeneratedMapInternal' is not valid"), *FString(__FUNCTION__));
	}
#endif // WITH_EDITOR
	return GeneratedMapInternal;
}

// The Generated Map setter
void UGeneratedMapSubsystem::SetGeneratedMap(AGeneratedMap* InGeneratedMap)
{
	if (!ensureMsgf(InGeneratedMap, TEXT("%s: 'InGeneratedMap' is not valid"), *FString(__FUNCTION__)))
	{
		return;
	}

	GeneratedMapInternal = InGeneratedMap;

	if (OnGeneratedMapReady.IsBound())
	{
		OnGeneratedMapReady.Broadcast(InGeneratedMap);
	}
}