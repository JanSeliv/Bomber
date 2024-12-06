// Copyright (c) Yevhenii Selivanov

#include "Components/NMMHUDComponent.h"
//---
#include "Data/NMMDataAsset.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Subsystems/WidgetsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "Widgets/NewMainMenuWidget.h"
#include "Widgets/NMMCinematicStateWidget.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMHUDComponent)

// Default constructor
UNMMHUDComponent::UNMMHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void UNMMHUDComponent::OnRegister()
{
	Super::OnRegister();

	// Listen to register widgets OnLocalCharacterReady to guarantee that the player controller is initialized, so we can use Widgets Subsystem
	BIND_ON_LOCAL_CHARACTER_READY(this, UNMMHUDComponent::OnLocalCharacterReady);
}

// Clears all transient data created by this component
void UNMMHUDComponent::OnUnregister()
{
	// --- Destroy Main Menu widgets

	if (UWidgetsSubsystem* WidgetsSubsystem = UWidgetsSubsystem::GetWidgetsSubsystem())
	{
		WidgetsSubsystem->DestroyManageableWidget(MainMenuWidgetInternal);
		WidgetsSubsystem->DestroyManageableWidget(InCinematicStateWidgetInternal);
	}

	MainMenuWidgetInternal = nullptr;
	InCinematicStateWidgetInternal = nullptr;

	Super::OnUnregister();
}

// Called when the local player character is spawned, possessed, and replicated
void UNMMHUDComponent::OnLocalCharacterReady_Implementation(class APlayerCharacter* Character, int32 CharacterID)
{
	// Create widgets now as fast as possible, later we will register them in Widgets Subsystem
	UWidgetsSubsystem& WidgetsSubsystem = UWidgetsSubsystem::Get();
	MainMenuWidgetInternal = WidgetsSubsystem.CreateManageableWidgetChecked<UNewMainMenuWidget>(UNMMDataAsset::Get().GetMainMenuWidgetData());
	InCinematicStateWidgetInternal = WidgetsSubsystem.CreateManageableWidgetChecked<UNMMCinematicStateWidget>(UNMMDataAsset::Get().GetInCinematicStateWidgetData());
}
