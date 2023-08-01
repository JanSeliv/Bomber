// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GeneratedMapSubsystem.h"
//---
#include "GeneratedMap.h"
//---
#if WITH_EDITOR
#include "Editor.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GeneratedMapSubsystem)

// Returns the Generated Map Subsystem, is checked and wil crash if can't be obtained
UGeneratedMapSubsystem& UGeneratedMapSubsystem::Get()
{
	UGeneratedMapSubsystem* GeneratedMapSubsystem = GetGeneratedMapSubsystem();
	checkf(GeneratedMapSubsystem, TEXT("%s: 'GeneratedMapSubsystem' is null"), *FString(__FUNCTION__));
	return *GeneratedMapSubsystem;
}

// Returns the pointer to the Generated Map Subsystem
UGeneratedMapSubsystem* UGeneratedMapSubsystem::GetGeneratedMapSubsystem()
{
	const UWorld* FoundWorld = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;

#if WITH_EDITOR
	if (!FoundWorld)
	{
		FoundWorld = FEditorUtilsLibrary::GetEditorWorld();
	}
#endif

	if (!ensureMsgf(FoundWorld, TEXT("%s: Can not obtain current world"), *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	return FoundWorld->GetSubsystem<UGeneratedMapSubsystem>();
}

// The Generated Map getter, nullptr otherwise
AGeneratedMap* UGeneratedMapSubsystem::GetGeneratedMap() const
{
#if WITH_EDITOR
	ensureMsgf(FEditorUtilsLibrary::IsCooking() || GeneratedMapInternal, TEXT("%s: [Editor] 'GeneratedMapInternal' is not valid"), *FString(__FUNCTION__));
#endif // WITH_EDITOR
	return GeneratedMapInternal;
}

// The Generated Map setter
void UGeneratedMapSubsystem::SetGeneratedMap(AGeneratedMap* InGeneratedMap)
{
	if (ensureMsgf(InGeneratedMap, TEXT("%s: 'InGeneratedMap' is not valid"), *FString(__FUNCTION__)))
	{
		GeneratedMapInternal = InGeneratedMap;
	}
}
