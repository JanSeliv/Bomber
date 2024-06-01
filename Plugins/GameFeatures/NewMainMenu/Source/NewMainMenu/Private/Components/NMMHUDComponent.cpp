// Copyright (c) Yevhenii Selivanov

#include "Components/NMMHUDComponent.h"
//---
#include "Data/NMMDataAsset.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
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

	constexpr int32 HighZOrder = 3;
	constexpr bool bAddToViewport = true;
	MainMenuWidgetInternal = FWidgetUtilsLibrary::CreateWidgetChecked<UNewMainMenuWidget>(UNMMDataAsset::Get().GetMainMenuWidgetClass(), bAddToViewport, HighZOrder);

	InCinematicStateWidgetInternal = FWidgetUtilsLibrary::CreateWidgetChecked<UNMMCinematicStateWidget>(UNMMDataAsset::Get().GetInCinematicStateWidgetClass());
}

// Clears all transient data created by this component
void UNMMHUDComponent::OnUnregister()
{
	// --- Destroy Main Menu widgets

	if (MainMenuWidgetInternal)
	{
		FWidgetUtilsLibrary::DestroyWidget(*MainMenuWidgetInternal);
		MainMenuWidgetInternal = nullptr;
	}

	if (InCinematicStateWidgetInternal)
	{
		FWidgetUtilsLibrary::DestroyWidget(*InCinematicStateWidgetInternal);
		InCinematicStateWidgetInternal = nullptr;
	}

	Super::OnUnregister();
}
