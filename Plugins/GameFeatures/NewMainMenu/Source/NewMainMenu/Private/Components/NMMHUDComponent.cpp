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
#include "DalSubsystem.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

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

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Overridable native event for when play begins for this component
void UNMMHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	UDalSubsystem::Get().ListenForDataAsset<UNMMDataAsset>(this, &ThisClass::OnDataAssetLoaded);
}

// Clears all transient data created by this component
void UNMMHUDComponent::OnUnregister()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	Super::OnUnregister();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the NMM data asset is loaded and available
void UNMMHUDComponent::OnDataAssetLoaded_Implementation(const UNMMDataAsset* DataAsset)
{
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::Player_LocalPawnReady, this, &ThisClass::OnLocalPawnReady);
}

// Called when the local player character is spawned, possessed, and replicated
void UNMMHUDComponent::OnLocalPawnReady_Implementation(const FGameplayEventData& Payload)
{
	if (UNMMPlayerControllerComponent* ControllerComponent = UNMMUtils::GetPlayerControllerComponent())
	{
		ControllerComponent->SetManagedInputContextsEnabled(UNMMBaseSubsystem::Get().GetCurrentMenuState());
	}
}
