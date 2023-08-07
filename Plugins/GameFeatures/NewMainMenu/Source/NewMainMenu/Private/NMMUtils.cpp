// Copyright (c) Yevhenii Selivanov

#include "NMMUtils.h"
//---
#include "UI/MyHUD.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "Components/NMMHUDComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMUtils)

// Returns the HUD component of the Main Menu
UNMMHUDComponent* UNMMUtils::GetHUDComponent()
{
	const AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD();
	return MyHUD ? MyHUD->FindComponentByClass<UNMMHUDComponent>() : nullptr;
}

// Returns the widget of the Main Menu.
UNewMainMenuWidget* UNMMUtils::GetMainMenuWidget()
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent();
	return HUDComponent ? HUDComponent->GetMainMenuWidget() : nullptr;
}

// Returns the widget of the In Cinematic State
UNMMCinematicStateWidget* UNMMUtils::GetInCinematicStateWidget()
{
	const UNMMHUDComponent* HUDComponent = GetHUDComponent();
	return HUDComponent ? HUDComponent->GetInCinematicStateWidget() : nullptr;
}
