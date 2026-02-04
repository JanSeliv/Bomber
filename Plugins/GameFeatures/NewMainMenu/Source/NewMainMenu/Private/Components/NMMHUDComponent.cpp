// Copyright (c) Yevhenii Selivanov

#include "Components/NMMHUDComponent.h"

// NMM
#include "Components/NMMPlayerControllerComponent.h"
#include "Data/NMMDataAsset.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Widgets/NMMCinematicStateWidget.h"
#include "Widgets/NewMainMenuWidget.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMHUDComponent)

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
	return WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag<UNewMainMenuWidget>(NmmGameplayTags::UI::Widget_Menu) : nullptr;
}

// Returns the In Cinematic State widget
UNMMCinematicStateWidget* UNMMHUDComponent::GetInCinematicStateWidget() const
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem();
	return WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag<UNMMCinematicStateWidget>(NmmGameplayTags::UI::Widget_Cinematic) : nullptr;
}

// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void UNMMHUDComponent::OnRegister()
{
	Super::OnRegister();

	// Listen to register widgets OnLocalPawnReady to guarantee that the player controller is initialized, so we can use Widgets Subsystem
	BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
}

// Clears all transient data created by this component
void UNMMHUDComponent::OnUnregister()
{
	// --- Destroy Main Menu widgets

	if (UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem())
	{
		WidgetsSubsystem->DestroyManageableWidgetByTag(NmmGameplayTags::UI::Widget_Menu);
		WidgetsSubsystem->DestroyManageableWidgetByTag(NmmGameplayTags::UI::Widget_Cinematic);
	}

	Super::OnUnregister();
}

// Called when the local player character is spawned, possessed, and replicated
void UNMMHUDComponent::OnLocalPawnReady_Implementation(const FGameplayEventData& Payload)
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
