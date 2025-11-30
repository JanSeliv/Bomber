// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrGeneratedMapSubsystem.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

// UE
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGeneratedMapSubsystem)

// Returns the Generated Map Subsystem, is checked and will crash if can't be obtained
UBmrGeneratedMapSubsystem& UBmrGeneratedMapSubsystem::Get(const UObject* WorldContextObject /* = nullptr*/)
{
	UBmrGeneratedMapSubsystem* GeneratedMapSubsystem = GetGeneratedMapSubsystem(WorldContextObject);
	checkf(GeneratedMapSubsystem, TEXT("%s: 'GeneratedMapSubsystem' is null"), *FString(__FUNCTION__));
	return *GeneratedMapSubsystem;
}

// Returns the pointer to the Generated Map Subsystem
UBmrGeneratedMapSubsystem* UBmrGeneratedMapSubsystem::GetGeneratedMapSubsystem(const UObject* WorldContextObject /* = nullptr*/)
{
	const UWorld* FoundWorld = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	return FoundWorld ? FoundWorld->GetSubsystem<UBmrGeneratedMapSubsystem>() : nullptr;
}

// The Generated Map getter, nullptr otherwise
ABmrGeneratedMap* UBmrGeneratedMapSubsystem::GetGeneratedMap(bool bWarnIfNull /* = true*/) const
{
#if WITH_EDITOR
	if (bWarnIfNull)
	{
		ensureMsgf(FEditorUtilsLibrary::IsCooking() || GeneratedMap, TEXT("%s: [Editor] 'GeneratedMap' is not valid"), *FString(__FUNCTION__));
	}
#endif // WITH_EDITOR
	return GeneratedMap;
}

// The Generated Map setter
void UBmrGeneratedMapSubsystem::SetGeneratedMap(ABmrGeneratedMap* InGeneratedMap)
{
	if (!ensureMsgf(InGeneratedMap, TEXT("%s: 'InGeneratedMap' is not valid"), *FString(__FUNCTION__)))
	{
		return;
	}

	GeneratedMap = InGeneratedMap;

	if (OnGeneratedMapReady.IsBound())
	{
		OnGeneratedMapReady.Broadcast(InGeneratedMap);
	}
}