// Copyright (c) Yevhenii Selivanov

#include "Components/NMMHUDComponent.h"
//---
#include "Data/NMMDataAsset.h"
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

	if (UWidgetsSubsystem* WidgetsSubsystem = UWidgetsSubsystem::GetWidgetsSubsystem())
	{
		constexpr int32 HighZOrder = 3;
		constexpr bool bAddToViewport = true;
		MainMenuWidgetInternal = WidgetsSubsystem->CreateManageableWidgetChecked<UNewMainMenuWidget>(UNMMDataAsset::Get().GetMainMenuWidgetClass(), bAddToViewport, HighZOrder);
		InCinematicStateWidgetInternal = WidgetsSubsystem->CreateManageableWidgetChecked<UNMMCinematicStateWidget>(UNMMDataAsset::Get().GetInCinematicStateWidgetClass());
	}
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
