// Copyright (c) Yevhenii Selivanov

#include "Components/NMMHUDComponent.h"
//---
#include "Data/NMMDataAsset.h"
#include "UI/MyHUD.h"
#include "Widgets/NMMCinematicStateWidget.h"
#include "Widgets/NewMainMenuWidget.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMHUDComponent)

// Default constructor
UNMMHUDComponent::UNMMHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns HUD actor of this component.
AMyHUD* UNMMHUDComponent::GetHUD() const
{
	return Cast<AMyHUD>(GetOwner());
}

AMyHUD& UNMMHUDComponent::GetHUDChecked() const
{
	AMyHUD* MyHUD = GetHUD();
	checkf(MyHUD, TEXT("%s: 'MyHUD' is null"), *FString(__FUNCTION__));
	return *MyHUD;
}

// Returns the data asset that contains all the assets and tweaks of New Main Menu game feature
const UNMMDataAsset* UNMMHUDComponent::GetNewMainMenuDataAsset() const
{
	return NewMainMenuDataAssetInternal.LoadSynchronous();
}

// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void UNMMHUDComponent::OnRegister()
{
	Super::OnRegister();

	checkf(!NewMainMenuDataAssetInternal.IsNull(), TEXT("ERROR: 'NewMainMenuDataAssetInternal' is null!"));
	const AMyHUD& HUD = GetHUDChecked();

	MainMenuWidgetInternal = HUD.CreateWidgetByClass<UNewMainMenuWidget>(GetNewMainMenuDataAsset()->GetMainMenuWidgetClass());
	InCinematicStateWidgetInternal = HUD.CreateWidgetByClass<UNMMCinematicStateWidget>(GetNewMainMenuDataAsset()->GetInCinematicStateWidgetClass());
}
