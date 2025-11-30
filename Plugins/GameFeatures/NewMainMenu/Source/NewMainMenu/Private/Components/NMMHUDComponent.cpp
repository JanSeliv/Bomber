// Copyright (c) Yevhenii Selivanov

#include "Components/NMMHUDComponent.h"

// NMM
#include "Components/NMMPlayerControllerComponent.h"
#include "Data/NMMDataAsset.h"
#include "NMMUtils.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Widgets/NMMCinematicStateWidget.h"
#include "Widgets/NewMainMenuWidget.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMHUDComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_WIDGET_NEWMAINMENU_MENU, TEXT("UI.Widget.NewMainMenu.Menu"));
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_WIDGET_NEWMAINMENU_CINEMATIC, TEXT("UI.Widget.NewMainMenu.Cinematic"));

// Default constructor
UNMMHUDComponent::UNMMHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns the Main Menu widget
UNewMainMenuWidget* UNMMHUDComponent::GetMainMenuWidget() const
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem();
	return WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag<UNewMainMenuWidget>(TAG_UI_WIDGET_NEWMAINMENU_MENU) : nullptr;
}

// Returns the In Cinematic State widget
UNMMCinematicStateWidget* UNMMHUDComponent::GetInCinematicStateWidget() const
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem();
	return WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag<UNMMCinematicStateWidget>(TAG_UI_WIDGET_NEWMAINMENU_CINEMATIC) : nullptr;
}

// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void UNMMHUDComponent::OnRegister()
{
	Super::OnRegister();

	// Listen to register widgets OnLocalPawnReady to guarantee that the player controller is initialized, so we can use Widgets Subsystem
	BIND_ON_LOCAL_PAWN_READY(this, UNMMHUDComponent::OnLocalPawnReady);
}

// Clears all transient data created by this component
void UNMMHUDComponent::OnUnregister()
{
	// --- Destroy Main Menu widgets

	if (UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem())
	{
		WidgetsSubsystem->DestroyManageableWidgetByTag(TAG_UI_WIDGET_NEWMAINMENU_MENU);
		WidgetsSubsystem->DestroyManageableWidgetByTag(TAG_UI_WIDGET_NEWMAINMENU_CINEMATIC);
	}

	Super::OnUnregister();
}

// Called when the local player character is spawned, possessed, and replicated
void UNMMHUDComponent::OnLocalPawnReady_Implementation(class ABmrPawn* Character, int32 PlayerId)
{
	UBmrWidgetsSubsystem::Get().CreateManageableWidgetChecked(UNMMDataAsset::Get().GetMainMenuWidgetData());
	UBmrWidgetsSubsystem::Get().CreateManageableWidgetChecked(UNMMDataAsset::Get().GetInCinematicStateWidgetData());

	// Once HUD is displayed, set the Menu state OnLocalPawnReady
	// It guarantee that game enters the Menu state only when the character is ready and HUD is displayed
	if (UNMMPlayerControllerComponent* ControllerComponent = UNMMUtils::GetPlayerControllerComponent())
	{
		ControllerComponent->TrySetMenuState();
		ControllerComponent->SetManagedInputContextsEnabled(UNMMBaseSubsystem::Get().GetCurrentMenuState());
	}
}
