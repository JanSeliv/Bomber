// Copyright (c) Yevhenii Selivanov

#include "Components/NewMainMenuHUDComponent.h"
//---
#include "NewMainMenuWidget.h"
#include "Data/NewMainMenuDataAsset.h"
#include "UI/MyHUD.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuHUDComponent)

// Default constructor
UNewMainMenuHUDComponent::UNewMainMenuHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Returns HUD actor of this component.
AMyHUD* UNewMainMenuHUDComponent::GetHUD() const
{
	return Cast<AMyHUD>(GetOwner());
}

AMyHUD& UNewMainMenuHUDComponent::GetHUDChecked() const
{
	AMyHUD* MyHUD = GetHUD();
	checkf(MyHUD, TEXT("%s: 'MyHUD' is null"), *FString(__FUNCTION__));
	return *MyHUD;
}

// Returns the data asset that contains all the assets and tweaks of New Main Menu game feature
const UNewMainMenuDataAsset* UNewMainMenuHUDComponent::GetNewMainMenuDataAsset() const
{
	return NewMainMenuDataAssetInternal.LoadSynchronous();
}

// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void UNewMainMenuHUDComponent::OnRegister()
{
	Super::OnRegister();

	MainMenuWidgetInternal = GetHUDChecked().CreateWidgetByClass<UNewMainMenuWidget>(UNewMainMenuDataAsset::Get().GetMainMenuWidgetClass());
}
