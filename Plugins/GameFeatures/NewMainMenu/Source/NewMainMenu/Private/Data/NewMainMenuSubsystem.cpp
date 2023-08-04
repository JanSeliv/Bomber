// Copyright (c) Yevhenii Selivanov

#include "Data/NewMainMenuSubsystem.h"
//---
#include "Components/NewMainMenuSpotComponent.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNewMainMenuSubsystem& UNewMainMenuSubsystem::Get()
{
	const UWorld* World = UMyBlueprintFunctionLibrary::GetStaticWorld();
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	UNewMainMenuSubsystem* ThisSubsystem = World->GetSubsystem<ThisClass>();
	checkf(ThisSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Add new Main-Menu spot, so it can be obtained by other objects
void UNewMainMenuSubsystem::AddNewMainMenuSpot(UNewMainMenuSpotComponent* NewMainMenuSpotComponent)
{
	if (ensureMsgf(NewMainMenuSpotComponent, TEXT("%s: 'NewMainMenuSpotComponent' is null"), *FString(__FUNCTION__)))
	{
		MainMenuSpotsInternal.AddUnique(NewMainMenuSpotComponent);
	}
}

// Returns currently selected Main-Menu spot
UNewMainMenuSpotComponent* UNewMainMenuSubsystem::GetActiveMainMenuSpotComponent() const
{
	for (UNewMainMenuSpotComponent* MainMenuSpotComponent : MainMenuSpotsInternal)
	{
		if (MainMenuSpotComponent && MainMenuSpotComponent->IsActive())
		{
			return MainMenuSpotComponent;
		}
	}

	return nullptr;
}
